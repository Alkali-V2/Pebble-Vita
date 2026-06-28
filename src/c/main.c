#include <pebble.h>

extern uint32_t MESSAGE_KEY_BG_COLOR;
extern uint32_t MESSAGE_KEY_FILLED_COLOR;
extern uint32_t MESSAGE_KEY_EMPTY_COLOR;
extern uint32_t MESSAGE_KEY_SHOW_DATE;
extern uint32_t MESSAGE_KEY_DATE_STYLE;

#define GRID_COLS 4
#define GRID_ROWS 3

#define PERSIST_KEY_BG          1
#define PERSIST_KEY_FILLED      2
#define PERSIST_KEY_EMPTY       3
#define PERSIST_KEY_SHOW_DATE   4
#define PERSIST_KEY_DATE_STYLE  5

#ifdef PBL_BW
#define DEFAULT_BG_ARGB     0xFF  // white
#define DEFAULT_FILLED_ARGB 0xC0  // black
#define DEFAULT_EMPTY_ARGB  0xEA  // light gray (dithered on B&W display)
#else
#define DEFAULT_BG_ARGB     0xFF
#define DEFAULT_FILLED_ARGB 0xD5
#define DEFAULT_EMPTY_ARGB  0xEA
#endif

static Window *s_window;
static Layer  *s_canvas_layer;

// Time state — updated every minute by tick_handler
static int     s_hours, s_minutes;
static uint8_t s_is_pm;

// Settings — loaded from persist at init, updated via AppMessage
static uint8_t s_bg_argb, s_filled_argb, s_empty_argb;
static uint8_t s_show_date, s_date_style;

// Layout cache — computed once at window_load, never change for a given device
static GFont s_font;
static int   s_radius, s_spacing, s_seg_w, s_seg_sp, s_bar_w, s_bar_h, s_font_h;
static int   s_grid_x, s_grid_y, s_bm, s_bar_y, s_text_y;
static int   s_dot_r, s_dot_cy;
static int   s_am_lx, s_am_dot_x, s_am_sz_w;
static int   s_pm_lx, s_pm_dot_x, s_pm_sz_w;

static uint8_t rgb24_to_argb8(int32_t val) {
    uint8_t r = (val >> 16) & 0xFF;
    uint8_t g = (val >> 8)  & 0xFF;
    uint8_t b =  val        & 0xFF;
    return (uint8_t)((3 << 6) | ((r >> 6) << 4) | ((g >> 6) << 2) | (b >> 6));
}

static GColor contrasting_color(uint8_t bg_argb) {
    int r = (bg_argb >> 4) & 0x3;
    int g = (bg_argb >> 2) & 0x3;
    int b =  bg_argb       & 0x3;
    return (r * 30 + g * 59 + b * 11 >= 150) ? GColorBlack : GColorWhite;
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    GColor bg     = (GColor){.argb = s_bg_argb};
    GColor filled = (GColor){.argb = s_filled_argb};
    GColor empty  = (GColor){.argb = s_empty_argb};

    // ── background ────────────────────────────────────────────────────────────
    graphics_context_set_fill_color(ctx, bg);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

    // ── hours grid ────────────────────────────────────────────────────────────
    int partial_index = s_hours + 1;
    
    for(int row = 0; row < GRID_ROWS; row++) {
      for(int col = 0; col < GRID_COLS; col++) {
        int index = col * GRID_ROWS + row + 1;
        int cx = s_grid_x + col * s_spacing;
        int cy = s_grid_y + row * s_spacing;
        GRect block = GRect(cx - s_radius, cy - s_radius, s_radius * 2, s_radius * 2);
        
        if(index < partial_index) {
          //fully elapsed hour
          graphics_context_set_fill_color(ctx, filled);
          graphics_fill_rect(ctx, block, 0, GCornerNone);
        } else if (index == partial_index && index <= GRID_COLS * GRID_ROWS) {
          //Progressively filling minutes cell
          int fill_h = (block.size.h * s_minutes) / 60;
          graphics_context_set_fill_color(ctx, empty);
          graphics_fill_rect(ctx, block, 0, GCornerNone);
          graphics_context_set_fill_color(ctx, filled);
          graphics_fill_rect(ctx, GRect(block.origin.x, block.origin.y, block.size.w, fill_h), 0, GCornerNone);
        } else {
          //not reached yet
          graphics_context_set_fill_color(ctx, empty);
          graphics_fill_rect(ctx, block, 0, GCornerNone);
        }
      }
    }

    // ── bottom row: date left, AM/PM right ────────────────────────────────────
    graphics_context_set_text_color(ctx, contrasting_color(s_bg_argb));

    if (s_show_date) {
        char buf[16];
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        static const char *fmts[] = { "%m/%d/%y", "%m/%d", "%d/%m/%y", "%d/%m" };
        strftime(buf, sizeof(buf), fmts[s_date_style < 4 ? s_date_style : 0], t);
        graphics_draw_text(ctx, buf, s_font,
                           GRect(s_bm, s_text_y, s_bar_w, s_font_h),
                           GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    }

    // AM/PM — right-aligned, layout (right to left): [AM][●]  [PM][●] | bm+bar_w
    graphics_draw_text(ctx, "AM", s_font,
        GRect(s_am_lx, s_text_y, s_am_sz_w + 2, s_font_h), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    graphics_context_set_fill_color(ctx, s_is_pm ? empty : filled);
    graphics_fill_circle(ctx, GPoint(s_am_dot_x, s_dot_cy), s_dot_r);

    graphics_draw_text(ctx, "PM", s_font,
        GRect(s_pm_lx, s_text_y, s_pm_sz_w + 2, s_font_h), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    graphics_context_set_fill_color(ctx, s_is_pm ? filled : empty);
    graphics_fill_circle(ctx, GPoint(s_pm_dot_x, s_dot_cy), s_dot_r);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
    Tuple *t;

    t = dict_find(iter, MESSAGE_KEY_BG_COLOR);
    if (t) s_bg_argb = rgb24_to_argb8(t->value->int32);

    t = dict_find(iter, MESSAGE_KEY_FILLED_COLOR);
    if (t) s_filled_argb = rgb24_to_argb8(t->value->int32);

    t = dict_find(iter, MESSAGE_KEY_EMPTY_COLOR);
    if (t) s_empty_argb = rgb24_to_argb8(t->value->int32);

    t = dict_find(iter, MESSAGE_KEY_SHOW_DATE);
    if (t) s_show_date = (uint8_t)t->value->int32;

    t = dict_find(iter, MESSAGE_KEY_DATE_STYLE);
    if (t) {
        uint8_t v = (t->type == TUPLE_CSTRING) ? (uint8_t)atoi(t->value->cstring) : (uint8_t)t->value->int32;
        s_date_style = (v < 4) ? v : 0;
    }

    layer_mark_dirty(s_canvas_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    s_hours   = tick_time->tm_hour % 12;
    s_minutes = tick_time->tm_min;
    s_is_pm   = (tick_time->tm_hour >= 12) ? 1 : 0;
    layer_mark_dirty(s_canvas_layer);
}

static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root);
    int w = bounds.size.w;
    int h = bounds.size.h;
    bool emery = (w >= 180);

    // Compute all layout constants once — never recalculated while the watch runs
    s_radius  = emery ? 19 : 12;
    s_spacing = emery ? 48 : 32;
    s_bar_h   = emery ? 18 : 14;
    int bar_gap = s_spacing - 2 * s_radius;
    s_font_h  = emery ? 22 : 18;

    int grid_w = (GRID_COLS - 1) * s_spacing + s_radius * 2;
    s_grid_x  = (w - grid_w) / 2 - 1 + s_radius;

    s_seg_w  = emery ? 2 : 1;
    s_seg_sp = s_seg_w + 1;
    s_bar_w  = 60 * s_seg_sp;
    s_bm     = (w - s_bar_w) / 2 - 1;

    int total_h = (GRID_ROWS - 1) * s_spacing + 2 * s_radius + bar_gap + s_bar_h + bar_gap / 2 + s_font_h;
    s_grid_y = (h - total_h) / 2 + s_radius + 2;
    s_bar_y  = s_grid_y + (GRID_ROWS - 1) * s_spacing + s_radius + bar_gap;
    s_text_y = s_bar_y + s_bar_h + bar_gap / 2 + 2;

    s_font   = emery ? fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD)
                     : fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    s_dot_r  = emery ? 4 : 3;
    s_dot_cy = s_text_y + s_font_h / 2;

    GSize am_sz = graphics_text_layout_get_content_size(
        "AM", s_font, GRect(0, 0, s_bar_w / 2, s_font_h), GTextOverflowModeWordWrap, GTextAlignmentLeft);
    GSize pm_sz = graphics_text_layout_get_content_size(
        "PM", s_font, GRect(0, 0, s_bar_w / 2, s_font_h), GTextOverflowModeWordWrap, GTextAlignmentLeft);
    s_am_sz_w = am_sz.w;
    s_pm_sz_w = pm_sz.w;

    s_pm_dot_x = s_bm + s_bar_w - s_dot_r;
    s_pm_lx    = s_pm_dot_x - s_dot_r - 3 - s_pm_sz_w;
    s_am_dot_x = s_pm_lx - 5 - s_dot_r;
    s_am_lx    = s_am_dot_x - s_dot_r - 3 - s_am_sz_w;

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
    layer_add_child(root, s_canvas_layer);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    s_hours   = t->tm_hour % 12;
    s_minutes = t->tm_min;
    s_is_pm   = (t->tm_hour >= 12) ? 1 : 0;
}

static void window_unload(Window *window) {
    layer_destroy(s_canvas_layer);
}

#define persist_read_or(key, def) \
    (persist_exists(key) ? (uint8_t)persist_read_int(key) : (uint8_t)(def))

static void init(void) {
    s_bg_argb     = persist_read_or(PERSIST_KEY_BG,         DEFAULT_BG_ARGB);
    s_filled_argb = persist_read_or(PERSIST_KEY_FILLED,     DEFAULT_FILLED_ARGB);
    s_empty_argb  = persist_read_or(PERSIST_KEY_EMPTY,      DEFAULT_EMPTY_ARGB);
    s_show_date   = persist_read_or(PERSIST_KEY_SHOW_DATE,  1);
    s_date_style  = persist_read_or(PERSIST_KEY_DATE_STYLE, 0);

    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload
    });
    window_stack_push(s_window, true);
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    app_message_register_inbox_received(inbox_received_handler);
    app_message_open(72, 0);
}

static void deinit(void) {
    persist_write_int(PERSIST_KEY_BG,         s_bg_argb);
    persist_write_int(PERSIST_KEY_FILLED,     s_filled_argb);
    persist_write_int(PERSIST_KEY_EMPTY,      s_empty_argb);
    persist_write_int(PERSIST_KEY_SHOW_DATE,  s_show_date);
    persist_write_int(PERSIST_KEY_DATE_STYLE, s_date_style);
    tick_timer_service_unsubscribe();
    window_destroy(s_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
    return 0;
}
