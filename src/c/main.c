#include <pebble.h>

extern uint32_t MESSAGE_KEY_BG_COLOR;
extern uint32_t MESSAGE_KEY_FILLED_COLOR;
extern uint32_t MESSAGE_KEY_EMPTY_COLOR;
extern uint32_t MESSAGE_KEY_SHOW_MINUTES_TEXT;
extern uint32_t MESSAGE_KEY_AM_TEXT_COLOR;
extern uint32_t MESSAGE_KEY_AM_BORDER_COLOR;
extern uint32_t MESSAGE_KEY_PM_TEXT_COLOR;
extern uint32_t MESSAGE_KEY_PM_BORDER_COLOR;
extern uint32_t MESSAGE_KEY_TOP_BAR_STYLE;
extern uint32_t MESSAGE_KEY_BAR1_COLOR;
extern uint32_t MESSAGE_KEY_BAR2_COLOR;
extern uint32_t MESSAGE_KEY_BAR3_COLOR;
extern uint32_t MESSAGE_KEY_BIRTH_YEAR;
extern uint32_t MESSAGE_KEY_BIRTH_MONTH;
extern uint32_t MESSAGE_KEY_BIRTH_DAY;
extern uint32_t MESSAGE_KEY_LIFE_EXPECTANCY_YEAR;
extern uint32_t MESSAGE_KEY_SHOW_BAR1_TEXT;
extern uint32_t MESSAGE_KEY_SHOW_BAR2_TEXT;
extern uint32_t MESSAGE_KEY_SHOW_LIFE_BAR_TEXT;
extern uint32_t MESSAGE_KEY_BAR_ORDER;

#define GRID_COLS 4
#define GRID_ROWS 3

// Top-left bar metric choices (MESSAGE_KEY_TOP_BAR_STYLE / s_top_bar_style)
#define TOP_BAR_DAY_OF_MONTH 0
#define TOP_BAR_DAY_OF_YEAR 1

// Which physical slot (left vs right) the Day vs Month bar renders into
// (MESSAGE_KEY_BAR_ORDER / s_bar_order). Doesn't touch which fraction/text/
// color belongs to "Day" or "Month" — just where on screen each lands.
#define BAR_ORDER_DAY_FIRST 0
#define BAR_ORDER_MONTH_FIRST 1

#define PERSIST_KEY_BG 1
#define PERSIST_KEY_FILLED 2
#define PERSIST_KEY_EMPTY 3
// 4 and 5 formerly held SHOW_DATE / DATE_STYLE (removed; verbose date dropped
// in favor of the three progress bars below). Left retired rather than
// reused so upgrading users don't get a stale value reinterpreted.
#define PERSIST_KEY_SHOW_MINUTES_TEXT 6
#define PERSIST_KEY_AM_TEXT_COLOR 7
#define PERSIST_KEY_AM_BORDER_COLOR 8
#define PERSIST_KEY_PM_TEXT_COLOR 9
#define PERSIST_KEY_PM_BORDER_COLOR 10
#define PERSIST_KEY_TOP_BAR_STYLE 11
#define PERSIST_KEY_BAR1_COLOR 12
#define PERSIST_KEY_BAR2_COLOR 13
#define PERSIST_KEY_BAR3_COLOR 14
#define PERSIST_KEY_BIRTH_YEAR 15
#define PERSIST_KEY_BIRTH_MONTH 16
#define PERSIST_KEY_BIRTH_DAY 17
#define PERSIST_KEY_LIFE_EXPECTANCY_YEAR 18
#define PERSIST_KEY_SHOW_BAR1_TEXT 19
#define PERSIST_KEY_SHOW_LIFE_BAR_TEXT 20
#define PERSIST_KEY_SHOW_BAR2_TEXT 21
#define PERSIST_KEY_BAR_ORDER 22

#define DEFAULT_AM_TEXT_ARGB 0xFF   // white
#define DEFAULT_AM_BORDER_ARGB 0xC0 // black
#define DEFAULT_PM_TEXT_ARGB 0xC0   // black
#define DEFAULT_PM_BORDER_ARGB 0xFF // white

#ifdef PBL_BW
#define DEFAULT_BG_ARGB 0xFF // white
#define DEFAULT_FILLED_ARGB 0xC0 // black
#define DEFAULT_EMPTY_ARGB 0xEA // light gray (dithered on B&W display)
#else
#define DEFAULT_BG_ARGB 0xFF
#define DEFAULT_FILLED_ARGB 0xD5
#define DEFAULT_EMPTY_ARGB 0xEA
#endif

// Bar defaults: day bar orange, month bar blue, life bar matches the
// "Active" grid color so it reads as part of the same family by default.
#define DEFAULT_BAR1_ARGB 0xE0       // orange
#define DEFAULT_BAR2_ARGB 0xC3       // blue
#define DEFAULT_BAR3_ARGB DEFAULT_FILLED_ARGB

#define DEFAULT_BIRTH_YEAR 1990
#define DEFAULT_BIRTH_MONTH 1
#define DEFAULT_BIRTH_DAY 1
#define DEFAULT_LIFE_EXPECTANCY_YEAR 2080

// Candidate fonts for the in-block minute number, largest to smallest.
// LECO fonts are digit-only and tabular (fixed digit width), which is exactly
// what we want for measuring/fitting a two-digit "00".."59" string.
static const char * const s_minutes_font_keys[] = {
    FONT_KEY_LECO_38_BOLD_NUMBERS,
    FONT_KEY_LECO_36_BOLD_NUMBERS,
    FONT_KEY_LECO_32_BOLD_NUMBERS,
    FONT_KEY_LECO_28_LIGHT_NUMBERS,
    FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM,
    FONT_KEY_LECO_20_BOLD_NUMBERS,
    FONT_KEY_GOTHIC_18_BOLD,
    FONT_KEY_GOTHIC_14_BOLD,
};

// 8 directions to nudge the outline draws — N, S, E, W, and diagonals for AM/PM Text
static const GPoint s_outline_offsets[8] = {
  {-1, -1}, {0, -1}, {1, -1},
  {-1,  0},           {1,  0},
  {-1,  1}, {0,  1}, {1,  1},
};

#define MINUTES_FONT_COUNT (sizeof(s_minutes_font_keys) / sizeof(s_minutes_font_keys[0]))

// Padding (px) kept clear inside each block's edge so the glyph never crosses
// the rounded-rect border. Nudge this up/down if it looks too tight/loose on
// your hardware.
#define MINUTES_TEXT_PADDING 3

// If the centered text sits a pixel or two high/low on your platform, adjust
// this fudge factor (in px, +down / -up) rather than the centering math.
#define MINUTES_TEXT_Y_FUDGE -4

// Same idea as the two constants above, but for the numbers drawn inside the
// progress bars (day number / age-over-total). Bars are thin, so padding is
// intentionally small; nudge BAR_TEXT_Y_FUDGE if the digits look off-center
// on your hardware.
#define BAR_TEXT_V_PADDING 1
#define BAR_TEXT_H_PADDING 4
#define BAR_TEXT_Y_FUDGE -2

static Window *s_window;
static Layer *s_canvas_layer;

// Time state — updated every minute by tick_handler
static int s_hours, s_minutes;
static uint8_t s_is_pm;

// Settings — loaded from persist at init, updated via AppMessage
static uint8_t s_bg_argb, s_filled_argb, s_empty_argb;
static uint8_t s_show_minutes_text;
static uint8_t s_am_text_argb, s_am_border_argb, s_pm_text_argb, s_pm_border_argb;
static uint8_t s_top_bar_style; // TOP_BAR_DAY_OF_MONTH or TOP_BAR_DAY_OF_YEAR
static uint8_t s_bar1_argb, s_bar2_argb, s_bar3_argb;
static int s_birth_year, s_birth_month, s_birth_day, s_life_expectancy_year;
static uint8_t s_show_bar1_text, s_show_bar2_text, s_show_life_bar_text;
static uint8_t s_top_bar_style; // TOP_BAR_DAY_OF_MONTH or TOP_BAR_DAY_OF_YEAR
static uint8_t s_bar_order;     // BAR_ORDER_DAY_FIRST or BAR_ORDER_MONTH_FIRST

// Text shown inside bar1 ("28" / "179") and the life bar ("40/80"),
// refreshed by recompute_bars() alongside the fractions they label.
static char s_bar1_text[8];
static char s_bar2_text[12];
static char s_life_bar_text[16];

// Bar progress cache — permille (0..1000), recomputed once per day (or
// whenever settings change) rather than on every minute tick, since none of
// these metrics need finer-than-daily resolution.
static int s_bar1_frac, s_bar2_frac, s_bar3_frac;
static int s_bars_computed_for_mday = -1; // last tm_mday the cache was built for

// Layout cache — computed once at window_load, never change for a given device
static int s_radius, s_spacing;
static int s_grid_x, s_grid_y;
static GRect s_bar1_rect, s_bar2_rect, s_life_bar_rect;
static GFont s_minutes_font;
static int s_minutes_text_h;
static GFont s_bar_text_font;
static int s_bar_text_h;

static uint8_t rgb24_to_argb8(int32_t val) {
  uint8_t r = (val >> 16) & 0xFF;
  uint8_t g = (val >> 8) & 0xFF;
  uint8_t b = val & 0xFF;
  return (uint8_t)((3 << 6) | ((r >> 6) << 4) | ((g >> 6) << 2) | (b >> 6));
}

static int clampi(int v, int lo, int hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

static int clamp_permille(int v) {
  return clampi(v, 0, 1000);
}

// Black or white, whichever reads better against a given background. Used
// for the bar-text "ink" — paired with an outline halo (see
// draw_outlined_text) so it stays legible over both the filled and empty
// portions of a bar, not just the page background.
static GColor contrasting_color(uint8_t bg_argb) {
  int r = (bg_argb >> 4) & 0x3;
  int g = (bg_argb >> 2) & 0x3;
  int b = bg_argb & 0x3;
  return (r * 30 + g * 59 + b * 11 >= 150) ? GColorBlack : GColorWhite;
}

// ── Calendar math (no floating point) ───────────────────────────────────────
static const int DAYS_IN_MONTH[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static bool is_leap_year(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int days_in_month(int year, int month) { // month 1..12
  if (month == 2 && is_leap_year(year)) return 29;
  return DAYS_IN_MONTH[month - 1];
}

static int days_in_year(int year) {
  return is_leap_year(year) ? 366 : 365;
}

static int day_of_year(int year, int month, int day) { // 1-based
  int d = day;
  for (int m = 1; m < month; m++) d += days_in_month(year, m);
  return d;
}

// Day count from a fixed base year; only differences between two calls are
// meaningful (used for age-in-days and lifespan-in-days).
static int absolute_days(int year, int month, int day) {
  int total = 0;
  for (int y = 1900; y < year; y++) total += days_in_year(y);
  return total + day_of_year(year, month, day);
}

// Recompute the three bar fractions from the current date + settings. Cheap,
// but deliberately NOT called every minute — only at startup, once per day,
// and right after the settings page saves a change.
static void recompute_bars(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int year = t->tm_year + 1900;
  int month = t->tm_mon + 1;
  int day = t->tm_mday;

  if (s_top_bar_style == TOP_BAR_DAY_OF_YEAR) {
    s_bar1_frac = (day_of_year(year, month, day) - 1) * 1000 / days_in_year(year);
  } else {
    s_bar1_frac = (day - 1) * 1000 / days_in_month(year, month);
  }

  s_bar2_frac = month * 1000 / 12;
  snprintf(s_bar2_text, sizeof(s_bar2_text), "%d", month);

  int age_days = absolute_days(year, month, day) -
                 absolute_days(s_birth_year, s_birth_month, s_birth_day);
  int life_days = absolute_days(s_life_expectancy_year, s_birth_month, s_birth_day) -
                   absolute_days(s_birth_year, s_birth_month, s_birth_day);
  if (life_days < 1) life_days = 1;
  s_bar3_frac = clamp_permille((age_days * 1000) / life_days);

  // Bar1's number mirrors whichever metric it's showing.
  if (s_top_bar_style == TOP_BAR_DAY_OF_YEAR) {
    snprintf(s_bar1_text, sizeof(s_bar1_text), "%d", day_of_year(year, month, day));
  } else {
    snprintf(s_bar1_text, sizeof(s_bar1_text), "%d", day);
  }

  // Life bar's number is a simple whole-years reading — current age over
  // total expected lifespan — separate from the day-accurate fill fraction
  // above. Clamped so a birth year typo can't print something nonsensical.
  int age_years = year - s_birth_year;
  if (month < s_birth_month || (month == s_birth_month && day < s_birth_day)) {
    age_years -= 1;
  }
age_years = clampi(age_years, 0, 999);
  int total_years = clampi(s_life_expectancy_year - s_birth_year, 1, 999);
  snprintf(s_life_bar_text, sizeof(s_life_bar_text), "%d/%d", age_years, total_years);

  s_bars_computed_for_mday = day;
}

// Fills `bar` with `empty`, then overlays the left `frac_permille/1000` of it
// with `fill`. No rounding — flat edges stay crisp at an 8-10px bar height.
static void fill_bar(GContext *ctx, GRect bar, int frac_permille, GColor fill, GColor empty) {
  graphics_context_set_fill_color(ctx, empty);
  graphics_fill_rect(ctx, bar, 0, GCornerNone);

  int frac = clamp_permille(frac_permille);
  int fill_w = (bar.size.w * frac) / 1000;
  if (fill_w > 0) {
    graphics_context_set_fill_color(ctx, fill);
    graphics_fill_rect(ctx, GRect(bar.origin.x, bar.origin.y, fill_w, bar.size.h), 0, GCornerNone);
  }
}

// Stamps `text` with an `outline` halo (the same 8-direction nudge trick used
// for the AM/PM minute number) then the `fill` ink on top, so the digits stay
// legible whether they land over a bar's filled or empty portion.
static void draw_outlined_text(GContext *ctx, const char *text, GFont font, GRect box,
                                GColor fill, GColor outline) {
  graphics_context_set_text_color(ctx, outline);
  for (int i = 0; i < 8; i++) {
    GRect ob = box;
    ob.origin.x += s_outline_offsets[i].x;
    ob.origin.y += s_outline_offsets[i].y;
    graphics_draw_text(ctx, text, font, ob, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
  graphics_context_set_text_color(ctx, fill);
  graphics_draw_text(ctx, text, font, box, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GColor bg = (GColor){.argb = s_bg_argb};
  GColor filled = (GColor){.argb = s_filled_argb};
  GColor empty = (GColor){.argb = s_empty_argb};

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
        graphics_fill_rect(ctx, block, 4, GCornersAll);
      } else if (index == partial_index && index <= GRID_COLS * GRID_ROWS) {
        //Progressively filling minutes cell
        int fill_h = (block.size.h * s_minutes) / 60;
        graphics_context_set_fill_color(ctx, empty);
        graphics_fill_rect(ctx, block, 4, GCornersAll);
        graphics_context_set_fill_color(ctx, filled);
        int overlay_radius = (fill_h < 4) ? fill_h : 4;
        graphics_fill_rect(ctx, GRect(block.origin.x, block.origin.y, block.size.w, fill_h), overlay_radius, GCornerTopLeft | GCornerTopRight);

        // Restore the block's correctly-rounded bottom corners in case the
        // filled overlay's square bottom edge has crept into that curve zone.
        int bottom_empty_h = block.size.h - fill_h;
        if (bottom_empty_h > 0) {
          int patch_h = (bottom_empty_h < 4) ? bottom_empty_h : 4;
          graphics_context_set_fill_color(ctx, empty);
          graphics_fill_rect(ctx,
              GRect(block.origin.x, block.origin.y + block.size.h - patch_h, block.size.w, patch_h),
              patch_h, GCornerBottomLeft | GCornerBottomRight);
        }
        
        if (s_show_minutes_text) {
          char mbuf[3];
          snprintf(mbuf, sizeof(mbuf), "%02d", s_minutes);

          // AM = light fill / dark outline, PM = dark fill / light outline —
          // the number itself now encodes AM/PM regardless of what's behind it.
          GColor fill_color = (GColor){ .argb = s_is_pm ? s_pm_text_argb : s_am_text_argb };
          GColor outline_color = (GColor){ .argb = s_is_pm ? s_pm_border_argb : s_am_border_argb };

          int ty = block.origin.y + (block.size.h - s_minutes_text_h) / 2 + MINUTES_TEXT_Y_FUDGE;
          GRect text_box = GRect(block.origin.x, ty, block.size.w, s_minutes_text_h + 4);

          graphics_context_set_text_color(ctx, outline_color);
          for (int i = 0; i < 8; i++) {
            GRect ob = text_box;
            ob.origin.x += s_outline_offsets[i].x;
            ob.origin.y += s_outline_offsets[i].y;
            graphics_draw_text(ctx, mbuf, s_minutes_font, ob, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
        }
          
          graphics_context_set_text_color(ctx, fill_color);
          graphics_draw_text(ctx, mbuf, s_minutes_font, text_box, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
        } 
      } else {
        //not reached yet
        graphics_context_set_fill_color(ctx, empty);
        graphics_fill_rect(ctx, block, 4, GCornersAll);
      }
    }
  }

// ── bottom: three progress bars ─────────────────────────────────────────────
  // s_bar1_frac/text/color is always "Day" and s_bar2_frac/text/color is
  // always "Month" — their colors and toggles keep the meaning their labels
  // promise. s_bar_order only decides which physical slot (left vs right)
  // each one lands in, so a settings change just swaps positions, not
  // identities. Bottom (full width): life remaining, unaffected by order.
  GRect day_rect   = (s_bar_order == BAR_ORDER_MONTH_FIRST) ? s_bar2_rect : s_bar1_rect;
  GRect month_rect = (s_bar_order == BAR_ORDER_MONTH_FIRST) ? s_bar1_rect : s_bar2_rect;

  fill_bar(ctx, day_rect, s_bar1_frac, (GColor){.argb = s_bar1_argb}, empty);
  fill_bar(ctx, month_rect, s_bar2_frac, (GColor){.argb = s_bar2_argb}, empty);
  fill_bar(ctx, s_life_bar_rect, s_bar3_frac, (GColor){.argb = s_bar3_argb}, empty);

  // Optional numbers inside bar1, bar2, and the life bar.
  if (s_show_bar1_text || s_show_bar2_text || s_show_life_bar_text) {
    GColor ink = contrasting_color(s_empty_argb);
    GColor halo = empty;

    if (s_show_bar1_text) {
      int ty = day_rect.origin.y + (day_rect.size.h - s_bar_text_h) / 2 + BAR_TEXT_Y_FUDGE;
      GRect box = GRect(day_rect.origin.x, ty, day_rect.size.w, s_bar_text_h + 4);
      draw_outlined_text(ctx, s_bar1_text, s_bar_text_font, box, ink, halo);
    }

    if (s_show_bar2_text) {
      int ty = month_rect.origin.y + (month_rect.size.h - s_bar_text_h) / 2 + BAR_TEXT_Y_FUDGE;
      GRect box = GRect(month_rect.origin.x, ty, month_rect.size.w, s_bar_text_h + 4);
      draw_outlined_text(ctx, s_bar2_text, s_bar_text_font, box, ink, halo);
    }

    if (s_show_life_bar_text) {
      int ty = s_life_bar_rect.origin.y + (s_life_bar_rect.size.h - s_bar_text_h) / 2 + BAR_TEXT_Y_FUDGE;
      GRect box = GRect(s_life_bar_rect.origin.x, ty, s_life_bar_rect.size.w, s_bar_text_h + 4);
      draw_outlined_text(ctx, s_life_bar_text, s_bar_text_font, box, ink, halo);
    }
  }
}

// Decode an integer tuple at the width PebbleKit JS actually sent it in. JS
// numbers arrive in the smallest signed type that fits (1/2/4 bytes), so
// reading value->int32 unconditionally can pull in adjacent bytes whenever
// fewer were sent — this mainly bites small values like a birth month or
// the bar-style index. Colors are unaffected (RGB24 values always need the
// full 4 bytes) but we route everything through here for consistency.
static int32_t tuple_to_int(const Tuple *t) {
  if (t->type == TUPLE_CSTRING) return atoi(t->value->cstring);
  switch (t->length) {
    case 1: return (t->type == TUPLE_UINT) ? t->value->uint8 : t->value->int8;
    case 2: return (t->type == TUPLE_UINT) ? t->value->uint16 : t->value->int16;
    default: return (t->type == TUPLE_UINT) ? (int32_t)t->value->uint32 : t->value->int32;
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *t;
  bool date_settings_changed = false;

  t = dict_find(iter, MESSAGE_KEY_BG_COLOR);
  if (t) s_bg_argb = rgb24_to_argb8(t->value->int32);

  t = dict_find(iter, MESSAGE_KEY_FILLED_COLOR);
  if (t) s_filled_argb = rgb24_to_argb8(t->value->int32);

  t = dict_find(iter, MESSAGE_KEY_EMPTY_COLOR);
  if (t) s_empty_argb = rgb24_to_argb8(t->value->int32);

  t = dict_find(iter, MESSAGE_KEY_SHOW_MINUTES_TEXT);
  if (t) s_show_minutes_text = (uint8_t)t->value->int32;
  
  t = dict_find(iter, MESSAGE_KEY_AM_TEXT_COLOR);
  if (t) s_am_text_argb = rgb24_to_argb8(t->value->int32);

  t = dict_find(iter, MESSAGE_KEY_AM_BORDER_COLOR);
  if (t) s_am_border_argb = rgb24_to_argb8(t->value->int32);

  t = dict_find(iter, MESSAGE_KEY_PM_TEXT_COLOR);
  if (t) s_pm_text_argb = rgb24_to_argb8(t->value->int32);

  t = dict_find(iter, MESSAGE_KEY_PM_BORDER_COLOR);
  if (t) s_pm_border_argb = rgb24_to_argb8(t->value->int32);

  t = dict_find(iter, MESSAGE_KEY_TOP_BAR_STYLE);
  if (t) {
    s_top_bar_style = (tuple_to_int(t) == TOP_BAR_DAY_OF_YEAR) ? TOP_BAR_DAY_OF_YEAR : TOP_BAR_DAY_OF_MONTH;
    date_settings_changed = true;
  }

  t = dict_find(iter, MESSAGE_KEY_BAR1_COLOR);
  if (t) s_bar1_argb = rgb24_to_argb8(t->value->int32);

  t = dict_find(iter, MESSAGE_KEY_BAR2_COLOR);
  if (t) s_bar2_argb = rgb24_to_argb8(t->value->int32);

  t = dict_find(iter, MESSAGE_KEY_BAR3_COLOR);
  if (t) s_bar3_argb = rgb24_to_argb8(t->value->int32);

  t = dict_find(iter, MESSAGE_KEY_BIRTH_YEAR);
  if (t) { s_birth_year = clampi(tuple_to_int(t), 1900, 2150); date_settings_changed = true; }

  t = dict_find(iter, MESSAGE_KEY_BIRTH_MONTH);
  if (t) { s_birth_month = clampi(tuple_to_int(t), 1, 12); date_settings_changed = true; }

  t = dict_find(iter, MESSAGE_KEY_BIRTH_DAY);
  if (t) { s_birth_day = clampi(tuple_to_int(t), 1, 31); date_settings_changed = true; }

  t = dict_find(iter, MESSAGE_KEY_LIFE_EXPECTANCY_YEAR);
  if (t) { s_life_expectancy_year = clampi(tuple_to_int(t), 1900, 2150); date_settings_changed = true; }

  t = dict_find(iter, MESSAGE_KEY_SHOW_BAR1_TEXT);
  if (t) s_show_bar1_text = (uint8_t)(tuple_to_int(t) != 0);
  
  t = dict_find(iter, MESSAGE_KEY_SHOW_BAR2_TEXT); 
  if (t) s_show_bar2_text = (uint8_t)(tuple_to_int(t) != 0);

  t = dict_find(iter, MESSAGE_KEY_SHOW_LIFE_BAR_TEXT);
  if (t) s_show_life_bar_text = (uint8_t)(tuple_to_int(t) != 0);
  
  t = dict_find(iter, MESSAGE_KEY_BAR_ORDER);
  if (t) {
    s_bar_order = (tuple_to_int(t) == BAR_ORDER_MONTH_FIRST) ? BAR_ORDER_MONTH_FIRST : BAR_ORDER_DAY_FIRST;
  }

  // The bar metrics only need recomputing when something that feeds them
  // changed — not on every settings save (e.g. a pure color tweak).
  if (date_settings_changed) recompute_bars();

  layer_mark_dirty(s_canvas_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_hours = tick_time->tm_hour % 12;
  s_minutes = tick_time->tm_min;
  s_is_pm = (tick_time->tm_hour >= 12) ? 1 : 0;

  // Day-granularity metrics only need recomputing once the day actually
  // rolls over — comparing the cached day-of-month is a cheap, foolproof way
  // to catch that without depending on exactly which TimeUnits flags a given
  // firmware reports alongside a minute tick.
  if (tick_time->tm_mday != s_bars_computed_for_mday) {
    recompute_bars();
  }

  layer_mark_dirty(s_canvas_layer);
}

// Pick the largest candidate font whose rendered "88" fits inside a block of
// side `block_side`, leaving MINUTES_TEXT_PADDING px clear on each edge.
static void pick_minutes_font(int block_side) {
  int max_dim = block_side - (2 * MINUTES_TEXT_PADDING);

  s_minutes_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD); // safe fallback
  s_minutes_text_h = 14;

  for (size_t i = 0; i < MINUTES_FONT_COUNT; i++) {
    GFont candidate = fonts_get_system_font(s_minutes_font_keys[i]);
    // Measure against a generously wide/tall box so the string never wraps
    // and we get its true single-line content size.
    GSize sz = graphics_text_layout_get_content_size(
        "88", candidate, GRect(0, 0, 200, 100),
        GTextOverflowModeFill, GTextAlignmentLeft);

    if (sz.w <= max_dim && sz.h <= max_dim) {
      s_minutes_font = candidate;
      s_minutes_text_h = sz.h;
      break;
    }
  }
}

// Candidates for the in-bar numbers, largest to smallest. GOTHIC_14 is the
// smallest system font Pebble ships (there's no GOTHIC_9/11), so this is as
// far down as we can fall back — it's why the bars need to be tall enough
// (>=14px) to give it room in the first place.
static const char * const s_bar_font_keys[] = {
    FONT_KEY_GOTHIC_18_BOLD,
    FONT_KEY_GOTHIC_18,
    FONT_KEY_GOTHIC_14_BOLD,
    FONT_KEY_GOTHIC_14,
};
#define BAR_FONT_COUNT (sizeof(s_bar_font_keys) / sizeof(s_bar_font_keys[0]))

// Picks the largest candidate that fits both bar1's number ("999", checked
// against bar1's narrower width) and the life bar's number ("999/999",
// checked against the life bar's full width) within `max_h`. One shared font
// keeps the two bars' digits visually consistent.
static void pick_bar_font(int max_h, int bar1_max_w, int life_max_w) {
  s_bar_text_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD); // safe fallback
  s_bar_text_h = 14;

  for (size_t i = 0; i < BAR_FONT_COUNT; i++) {
    GFont candidate = fonts_get_system_font(s_bar_font_keys[i]);
    GSize sz1 = graphics_text_layout_get_content_size(
        "999", candidate, GRect(0, 0, 200, 100), GTextOverflowModeFill, GTextAlignmentLeft);
    GSize sz2 = graphics_text_layout_get_content_size(
        "999/999", candidate, GRect(0, 0, 200, 100), GTextOverflowModeFill, GTextAlignmentLeft);

    if (sz1.h <= max_h && sz2.h <= max_h && sz1.w <= bar1_max_w && sz2.w <= life_max_w) {
      s_bar_text_font = candidate;
      s_bar_text_h = (sz1.h > sz2.h) ? sz1.h : sz2.h;
      break;
    }
  }
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  int w = bounds.size.w;
  int h = bounds.size.h;
  bool emery = (w >= 180);

  // Compute all layout constants once — never recalculated while the watch runs
  s_radius = emery ? 19 : 12;
  s_spacing = emery ? 48 : 32;
  int bar_gap = s_spacing - 2 * s_radius;   // vertical breathing room, scaled to platform
  int bar_h = 18;                           // progress bar thickness — fixed (not platform-
                                             // scaled) so the in-bar numbers get the same
                                             // font size everywhere; comfortably over the
                                             // 14px floor needed to fit GOTHIC_14 text.
  int bar_sub_gap = emery ? 8 : 6;          // gap between the two top bars

  int grid_w = (GRID_COLS - 1) * s_spacing + s_radius * 2;
  s_grid_x = (w - grid_w) / 2 - 1 + s_radius;
  int grid_left = s_grid_x - s_radius; // left edge the bars line up with

  int total_h = (GRID_ROWS - 1) * s_spacing + 2 * s_radius // grid
              + bar_gap + bar_h                            // gap + top bar row
              + bar_gap / 2 + bar_h;                        // gap + life bar row
  s_grid_y = (h - total_h) / 2 + s_radius + 2;

  int top_bars_y = s_grid_y + (GRID_ROWS - 1) * s_spacing + s_radius + bar_gap;
  int life_bar_y = top_bars_y + bar_h + bar_gap / 2;

  int bar1_w = (grid_w - bar_sub_gap) / 2;
  s_bar1_rect = GRect(grid_left, top_bars_y, bar1_w, bar_h);
  s_bar2_rect = GRect(grid_left + bar1_w + bar_sub_gap, top_bars_y,
                       grid_w - bar1_w - bar_sub_gap, bar_h); // absorbs rounding remainder
  s_life_bar_rect = GRect(grid_left, life_bar_y, grid_w, bar_h);

  // Largest minute-number font that fits inside one grid block without
  // crossing its rounded border.
  pick_minutes_font(s_radius * 2);

  // Largest bar-number font that fits inside the (now taller) bars.
  pick_bar_font(bar_h - 2 * BAR_TEXT_V_PADDING,
                s_bar1_rect.size.w - 2 * BAR_TEXT_H_PADDING,
                s_life_bar_rect.size.w - 2 * BAR_TEXT_H_PADDING);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root, s_canvas_layer);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  s_hours = t->tm_hour % 12;
  s_minutes = t->tm_min;
  s_is_pm = (t->tm_hour >= 12) ? 1 : 0;
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

#define persist_read_or(key, def) \
  (persist_exists(key) ? (uint8_t)persist_read_int(key) : (uint8_t)(def))
#define persist_read_int_or(key, def) \
  (persist_exists(key) ? persist_read_int(key) : (int)(def))

static void init(void) {
  s_bg_argb = persist_read_or(PERSIST_KEY_BG, DEFAULT_BG_ARGB);
  s_filled_argb = persist_read_or(PERSIST_KEY_FILLED, DEFAULT_FILLED_ARGB);
  s_empty_argb = persist_read_or(PERSIST_KEY_EMPTY, DEFAULT_EMPTY_ARGB);
  s_show_minutes_text = persist_read_or(PERSIST_KEY_SHOW_MINUTES_TEXT, 1);
  s_am_text_argb = persist_read_or(PERSIST_KEY_AM_TEXT_COLOR, DEFAULT_AM_TEXT_ARGB);
  s_am_border_argb = persist_read_or(PERSIST_KEY_AM_BORDER_COLOR, DEFAULT_AM_BORDER_ARGB);
  s_pm_text_argb = persist_read_or(PERSIST_KEY_PM_TEXT_COLOR, DEFAULT_PM_TEXT_ARGB);
  s_pm_border_argb = persist_read_or(PERSIST_KEY_PM_BORDER_COLOR, DEFAULT_PM_BORDER_ARGB);
  s_top_bar_style = persist_read_or(PERSIST_KEY_TOP_BAR_STYLE, TOP_BAR_DAY_OF_MONTH);
  s_bar1_argb = persist_read_or(PERSIST_KEY_BAR1_COLOR, DEFAULT_BAR1_ARGB);
  s_bar2_argb = persist_read_or(PERSIST_KEY_BAR2_COLOR, DEFAULT_BAR2_ARGB);
  s_bar3_argb = persist_read_or(PERSIST_KEY_BAR3_COLOR, DEFAULT_BAR3_ARGB);
  s_birth_year = persist_read_int_or(PERSIST_KEY_BIRTH_YEAR, DEFAULT_BIRTH_YEAR);
  s_birth_month = persist_read_int_or(PERSIST_KEY_BIRTH_MONTH, DEFAULT_BIRTH_MONTH);
  s_birth_day = persist_read_int_or(PERSIST_KEY_BIRTH_DAY, DEFAULT_BIRTH_DAY);
  s_life_expectancy_year = persist_read_int_or(PERSIST_KEY_LIFE_EXPECTANCY_YEAR, DEFAULT_LIFE_EXPECTANCY_YEAR);
  s_show_bar1_text = persist_read_or(PERSIST_KEY_SHOW_BAR1_TEXT, 1);
  s_show_bar2_text = persist_read_or(PERSIST_KEY_SHOW_BAR2_TEXT, 1);
  s_show_life_bar_text = persist_read_or(PERSIST_KEY_SHOW_LIFE_BAR_TEXT, 1);
  s_bar_order = persist_read_or(PERSIST_KEY_BAR_ORDER, BAR_ORDER_DAY_FIRST);

  recompute_bars();

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
      .load = window_load,
      .unload = window_unload
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(512, 64);
}

static void deinit(void) {
  persist_write_int(PERSIST_KEY_BG, s_bg_argb);
  persist_write_int(PERSIST_KEY_FILLED, s_filled_argb);
  persist_write_int(PERSIST_KEY_EMPTY, s_empty_argb);
  persist_write_int(PERSIST_KEY_SHOW_MINUTES_TEXT, s_show_minutes_text);
  persist_write_int(PERSIST_KEY_AM_TEXT_COLOR, s_am_text_argb);
  persist_write_int(PERSIST_KEY_AM_BORDER_COLOR, s_am_border_argb);
  persist_write_int(PERSIST_KEY_PM_TEXT_COLOR, s_pm_text_argb);
  persist_write_int(PERSIST_KEY_PM_BORDER_COLOR, s_pm_border_argb);
  persist_write_int(PERSIST_KEY_TOP_BAR_STYLE, s_top_bar_style);
  persist_write_int(PERSIST_KEY_BAR1_COLOR, s_bar1_argb);
  persist_write_int(PERSIST_KEY_BAR2_COLOR, s_bar2_argb);
  persist_write_int(PERSIST_KEY_BAR3_COLOR, s_bar3_argb);
  persist_write_int(PERSIST_KEY_BIRTH_YEAR, s_birth_year);
  persist_write_int(PERSIST_KEY_BIRTH_MONTH, s_birth_month);
  persist_write_int(PERSIST_KEY_BIRTH_DAY, s_birth_day);
  persist_write_int(PERSIST_KEY_LIFE_EXPECTANCY_YEAR, s_life_expectancy_year);
  persist_write_int(PERSIST_KEY_SHOW_BAR1_TEXT, s_show_bar1_text);
  persist_write_int(PERSIST_KEY_SHOW_BAR2_TEXT, s_show_bar2_text);
  persist_write_int(PERSIST_KEY_SHOW_LIFE_BAR_TEXT, s_show_life_bar_text);
  persist_write_int(PERSIST_KEY_BAR_ORDER, s_bar_order);

  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}