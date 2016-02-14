#pragma once

#define APP_KEY_MSG_TYPE 0
#define NO_DELTA_VALUE 65536

enum {
  MSG_TYPE_ERROR,
  MSG_TYPE_DATA,
  MSG_TYPE_PREFERENCES,
};

enum {
  __APP_KEYS_FOR_DATA,
  APP_KEY_RECENCY,
  APP_KEY_SGV_COUNT,
  APP_KEY_SGVS,
  APP_KEY_LAST_SGV,
  APP_KEY_TREND,
  APP_KEY_DELTA,
  APP_KEY_STATUS_TEXT,
  APP_KEY_BOLUSES,
};

enum {
  __APP_KEYS_FOR_PREFERENCES,
  APP_KEY_MMOL,
  APP_KEY_TOP_OF_GRAPH,
  APP_KEY_TOP_OF_RANGE,
  APP_KEY_BOTTOM_OF_RANGE,
  APP_KEY_BOTTOM_OF_GRAPH,
  APP_KEY_H_GRIDLINES,
  APP_KEY_BATTERY_AS_NUMBER,
  APP_KEY_TIME_ALIGN,
  APP_KEY_BATTERY_LOC,
  APP_KEY_NUM_ELEMENTS,
  APP_KEY_ELEMENTS,
};

enum {
  ALIGN_LEFT,
  ALIGN_CENTER,
  ALIGN_RIGHT,
};

enum {
  BATTERY_LOC_NONE,
  BATTERY_LOC_STATUS_RIGHT,
  BATTERY_LOC_TIME_TOP_LEFT,
  BATTERY_LOC_TIME_TOP_RIGHT,
  BATTERY_LOC_TIME_BOTTOM_LEFT,
  BATTERY_LOC_TIME_BOTTOM_RIGHT,
};
