module.exports = [
  {
    "type": "heading",
    "defaultValue": "Nooka"
  },

  // ── Colors ──────────────────────────────────────────────────────────────────
  {
    "type": "color",
    "allowGray": true,
    "messageKey": "BG_COLOR",
    "label": "Background",
    "defaultValue": "FFFFFF"
  },
  {
    "type": "color",
    "allowGray": true,
    "messageKey": "FILLED_COLOR",
    "label": "Active (dots & bar)",
    "defaultValue": "555555"
  },
  {
    "type": "color",
    "allowGray": true,
    "messageKey": "EMPTY_COLOR",
    "label": "Inactive (unfilled)",
    "defaultValue": "AAAAAA"
  },

  // ── Display ──────────────────────────────────────────────────────────────────
  {
    "type": "heading",
    "defaultValue": "Display"
  },
  {
    "type": "toggle",
    "messageKey": "SHOW_MINUTES_TEXT",
    "label": "Show Minute Number",
    "defaultValue": true
  },
  {
    "type": "color",
    "allowGray": true,
    "messageKey": "AM_TEXT_COLOR",
    "label": "AM Minute Text",
    "defaultValue": "FFFFFF"
  },
  {
    "type": "color",
    "allowGray": true,
    "messageKey": "AM_BORDER_COLOR",
    "label": "AM Minute Border",
    "defaultValue": "000000"
  },
  {
    "type": "color",
    "allowGray": true,
    "messageKey": "PM_TEXT_COLOR",
    "label": "PM Minute Text",
    "defaultValue": "000000"
  },
  {
    "type": "color",
    "allowGray": true,
    "messageKey": "PM_BORDER_COLOR",
    "label": "PM Minute Border",
    "defaultValue": "FFFFFF"
  },

  // ── Progress Bars ────────────────────────────────────────────────────────────
  {
    "type": "heading",
    "defaultValue": "Progress Bars"
  },
  {
    "type": "text",
    "defaultValue": "Two small bars sit above one long bar at the bottom of the face."
  },
  
  {
    "type": "select",
    "messageKey": "BAR_ORDER",
    "label": "Top Row Order",
    "defaultValue": "0",
    "options": [
      { "label": "Day, Month", "value": "0" },
      { "label": "Month, Day", "value": "1" }
    ]
  },
  {
    "type": "select",
    "messageKey": "TOP_BAR_STYLE",
    "label": "Day Bar Shows",
    "defaultValue": "0",
    "options": [
      { "label": "Day of Month", "value": "0" },
      { "label": "Day of Year",  "value": "1" }
    ]
  },
  
  {
    "type": "select",
    "messageKey": "TOP_BAR_STYLE",
    "label": "Top-Left Bar Shows",
    "defaultValue": "0",
    "options": [
      { "label": "Day of Month", "value": "0" },
      { "label": "Day of Year",  "value": "1" }
    ]
  },
  {
    "type": "toggle",
    "messageKey": "SHOW_BAR1_TEXT",
    "label": "Show Day Number on Bar",
    "defaultValue": true
  },
  
  {
  "type": "toggle",
  "messageKey": "SHOW_BAR2_TEXT",
  "label": "Show Month Number on Bar",
  "defaultValue": true
  },
  
  {
    "type": "color",
    "allowGray": true,
    "messageKey": "BAR1_COLOR",
    "label": "Day Bar Color (top-left)",
    "defaultValue": "FFAA00"
  },
  {
    "type": "color",
    "allowGray": true,
    "messageKey": "BAR2_COLOR",
    "label": "Month Bar Color (top-right)",
    "defaultValue": "0000FF"
  },
  {
    "type": "color",
    "allowGray": true,
    "messageKey": "BAR3_COLOR",
    "label": "Life Bar Color (bottom)",
    "defaultValue": "555555"
  },

  // ── Life Progress ────────────────────────────────────────────────────────────
  {
    "type": "heading",
    "defaultValue": "Life Progress"
  },
  {
    "type": "text",
    "defaultValue": "The bottom bar fills from your birthday toward the year you enter below."
  },
  {
    "type": "input",
    "messageKey": "BIRTH_YEAR",
    "label": "Birth Year",
    "attributes": { "type": "number", "min": 1900, "max": 2150 },
    "defaultValue": "1990"
  },
  {
    "type": "input",
    "messageKey": "BIRTH_MONTH",
    "label": "Birth Month (1-12)",
    "attributes": { "type": "number", "min": 1, "max": 12 },
    "defaultValue": "1"
  },
  {
    "type": "input",
    "messageKey": "BIRTH_DAY",
    "label": "Birth Day (1-31)",
    "attributes": { "type": "number", "min": 1, "max": 31 },
    "defaultValue": "1"
  },
  {
    "type": "input",
    "messageKey": "LIFE_EXPECTANCY_YEAR",
    "label": "Live Until Year",
    "attributes": { "type": "number", "min": 1900, "max": 2150 },
    "defaultValue": "2080"
  },
  {
    "type": "toggle",
    "messageKey": "SHOW_LIFE_BAR_TEXT",
    "label": "Show Age / Total Years on Bar",
    "defaultValue": true
  },

  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];