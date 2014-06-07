#include <pebble.h>
#include <time.h>

static const int HOUR_RADIUS = 40;
static const int MINUTE_RADIUS = 60;

static const GColor WINDOW_COLOR = GColorBlack;
static const GPoint center_point = { 84, 72 };

static Window *window;
static Layer *Hour;
static Layer *HourLine;
static Layer *Minute;
static Layer *MinuteLine;

static GPathInfo HOUR_SHAPES[4] = {
  {
    .num_points = 3,
    .points = (GPoint []) {{0, 40}, {35, -20}, {-35, -20}}
  },
  {
    .num_points = 4,
    .points = (GPoint []) {{0, 40}, {40, 0}, {0, -40}, {-40, 0}}
  },
  {
    .num_points = 5,
    .points = (GPoint []) {{0, 40}, {44, -13}, {24, -42}, {-24, -42}, {-44, -13}}
  },
  {
    .num_points = 6,
    .points = (GPoint []) {{0, 40}, {34, 20}, {34, -20}, {0, -40}, {-34, -20}, {-34, 20}}
  }
};

static GPathInfo MINUTE_SHAPES[4] = {
  {
    .num_points = 3,
    .points = (GPoint []) {{0, 60}, {52, -30}, {-52, -30}}
  },
  {
    .num_points = 4,
    .points = (GPoint []) {{0, 60}, {60, 0}, {0, -60}, {-60, 0}}
  },
  {
    .num_points = 5,
    .points = (GPoint []) {{0, 60}, {66, -17}, {36, -63}, {-36, -63}, {-66, -17}}
  },
  {
    .num_points = 6,
    .points = (GPoint []) {{0, 60}, {51, 30}, {51, -30}, {0, -60}, {-51, -30}, {-51, 30}}
  }
};

static void draw_shape(GContext* ctx, int shape, int32_t angle, bool hour) {
  if (shape < 3) {
    graphics_draw_circle(ctx, center_point, hour ? HOUR_RADIUS : MINUTE_RADIUS);
  } else {
    GPathInfo* path_info = (hour ? HOUR_SHAPES : MINUTE_SHAPES) + shape - 3;
    GPath* path = gpath_create(path_info);
    gpath_rotate_to(path, angle);
    gpath_move_to(path, center_point);
    gpath_draw_outline(ctx, path);
  }
}

static int get_shape(int32_t angle) {
  int shape = (angle * 12 / TRIG_MAX_ANGLE) / 2 + 3;
  if (shape >= 7) shape = -1;
  return shape;
}

static GPoint calculate_angled_point(int32_t angle, int radius) {
  GPoint pt;

  pt.y = 84 + (-cos_lookup(angle) * radius / TRIG_MAX_RATIO);
  pt.x = 72 + (sin_lookup(angle) * radius / TRIG_MAX_RATIO);

  return pt;
}

static int32_t time_to_hour_angle(struct tm *tick_time) {
  return TRIG_MAX_ANGLE * ((tick_time->tm_hour % 12) * 60 + tick_time->tm_min) / 720;
}

static int32_t time_to_minute_angle(struct tm *tick_time) {
  return TRIG_MAX_ANGLE * ((tick_time->tm_min * 60) + tick_time->tm_sec) / 3600;
}

static void hour_line_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int32_t hour_angle = time_to_hour_angle(t);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Hour update at %d", (int) hour_angle);
  graphics_draw_line(ctx, center_point, calculate_angled_point(hour_angle, HOUR_RADIUS)); // TODO
}

static void minute_line_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int32_t minute_angle = time_to_minute_angle(t);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Minute update at %d", (int) minute_angle);
  graphics_draw_line(ctx, center_point, calculate_angled_point(minute_angle, MINUTE_RADIUS)); // TODO
}

static void hour_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int32_t hour_angle = time_to_hour_angle(t);
  int shape = get_shape(hour_angle); //TODO
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Hour shape is %d", shape);

  // draw_shape(ctx, shape, hour_angle, true); //TODO
}

static void minute_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int32_t minute_angle = time_to_minute_angle(t);
  int shape = get_shape(minute_angle); //TODO
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Minute shape is %d", shape);

  // draw_shape(ctx, shape, minute_angle, false); //TODO
}

static void tick_minute_handler(struct tm *tick_time, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Tick");
  layer_mark_dirty(window_get_root_layer(window));
}

static void window_load(Window *window) {
  window_set_background_color(window, WINDOW_COLOR);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = GRect(0, 0, 144, 168);

  Hour = layer_create(bounds);
  HourLine = layer_create(bounds);
  Minute = layer_create(bounds);
  MinuteLine = layer_create(bounds);

  layer_set_update_proc(Hour, hour_update_proc);
  layer_set_update_proc(HourLine, hour_line_update_proc);
  layer_set_update_proc(Minute, minute_update_proc);
  layer_set_update_proc(MinuteLine, minute_line_update_proc);

  layer_add_child(window_layer, Hour);
  layer_add_child(window_layer, HourLine);
  layer_add_child(Hour, Minute);
  layer_add_child(Minute, MinuteLine);
}

static void window_unload(Window *window) {
  layer_destroy(Hour);
  layer_destroy(HourLine);
  layer_destroy(Minute);
  layer_destroy(MinuteLine);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  tick_timer_service_subscribe(SECOND_UNIT, tick_minute_handler);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
