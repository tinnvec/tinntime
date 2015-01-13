#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS_ICON 1
#define KEY_CONDITIONS 2
#define KEY_LOCATION 3
#define KEY_INVERT 4
#define KEY_UPDATE 5
#define KEY_VIBRATE 6

static Window *s_main_window;

static BitmapLayer *s_conditions_icon_layer;
static TextLayer *s_temperature_layer;
static TextLayer *s_conditions_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_location_layer;
static TextLayer *s_pebble_battery_layer;
static Layer *s_background_layer;
static Layer *s_pebble_battery_visual_layer;
static InverterLayer *s_inversion_layer;

static GFont s_temperature_font;
static GFont s_time_font;
static GFont s_date_font;
static GFont s_battery_font;

static GBitmap *s_conditions_day_clear_bitmap;
static GBitmap *s_conditions_night_clear_bitmap;
static GBitmap *s_conditions_day_few_clouds_bitmap;
static GBitmap *s_conditions_night_few_clouds_bitmap;
static GBitmap *s_conditions_clouds_bitmap;
static GBitmap *s_conditions_rain_bitmap;
static GBitmap *s_conditions_thunderstorm_bitmap;
static GBitmap *s_conditions_snow_bitmap;
static GBitmap *s_conditions_fog_bitmap;

BatteryChargeState battery_state;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char time_buffer[] = "--:--";
  static char date_buffer[] = "Wed, September 30";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(time_buffer, sizeof("--:--"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(time_buffer, sizeof("--:--"), "%I:%M", tick_time);
  }
  strftime(date_buffer, sizeof("Wed, September 30"), "%a, %B %e", tick_time);

  // Display the time on the TextLayer
  text_layer_set_text(s_time_layer, time_buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather update every 30 minutes
  if(tick_time->tm_min % (int)persist_read_int(KEY_UPDATE) == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
    // send the message!
    app_message_outbox_send();
  }
}

static void background_update_proc(Layer *current_layer, GContext *ctx) {
  GRect rect;
  rect.origin = GPoint(0, 0);
  rect.size = GSize(144, 168);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, rect, 0, GCornerNone);
}

static void battery_update_proc(Layer *current_layer, GContext *ctx) {
  GRect rect;
  rect.origin = GPoint(0, 156);
  rect.size = GSize((int)(115 * ((double)battery_state.charge_percent / 100)), 12);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, rect, 0, GCornerNone);
}

static void update_battery() {
  static char battery_buffer[] = "100%";
  text_layer_set_size(s_pebble_battery_layer, GSize(((int)(115 * ((double)battery_state.charge_percent / 100)) + 29), 16));
  snprintf(battery_buffer, sizeof(battery_buffer), "%d%%", battery_state.charge_percent);
  text_layer_set_text(s_pebble_battery_layer, battery_buffer);
  layer_mark_dirty(s_pebble_battery_visual_layer);
}

static void pebble_battery_handler(BatteryChargeState new_state) {
  battery_state = new_state;
  update_battery();
}

static void bluetooth_connection_handler(bool connected) {
  if(!connected && persist_write_bool(KEY_VIBRATE)) {
    vibes_long_pulse();
  }
}

static void set_font_style(TextLayer *current_layer, GFont new_font, GTextAlignment new_alignment, GColor new_background_color, GColor new_color) {
  text_layer_set_font(current_layer, new_font);
  text_layer_set_text_alignment(current_layer, new_alignment);
  text_layer_set_background_color(current_layer, new_background_color);
  text_layer_set_text_color(current_layer, new_color);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  int time_font_height = 51;
  int date_font_height = 28;

  // Create GFonts
  s_temperature_font = fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
  s_time_font = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);
  s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  s_battery_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);

  // Create GBitmaps
  s_conditions_day_clear_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_CLEAR);
  s_conditions_night_clear_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NIGHT_CLEAR);
  s_conditions_day_few_clouds_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_FEW_CLOUDS);
  s_conditions_night_few_clouds_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NIGHT_FEW_CLOUDS);
  s_conditions_clouds_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOUDS);
  s_conditions_rain_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RAIN);
  s_conditions_thunderstorm_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_THUNDERSTORM);
  s_conditions_snow_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SNOW);
  s_conditions_fog_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOG);

  // Create Background Layer
  s_background_layer = layer_create(window_bounds);
  layer_set_update_proc(s_background_layer, background_update_proc);

  // Create conditions icon Layer
  s_conditions_icon_layer = bitmap_layer_create(GRect(3, 3, 42, 42));
  bitmap_layer_set_alignment(s_conditions_icon_layer, GAlignTop);

  // Create temperature layer
  s_temperature_layer = text_layer_create(GRect(48, -4, 96, 44));
  set_font_style(s_temperature_layer, s_temperature_font, GTextAlignmentRight, GColorClear, GColorWhite);
  text_layer_set_text(s_temperature_layer, "--°");

  // Create conditions layer
  s_conditions_layer = text_layer_create(GRect(window_bounds.origin.x, 37, window_bounds.size.w, date_font_height));
  set_font_style(s_conditions_layer, s_date_font, GTextAlignmentCenter, GColorClear, GColorWhite);

  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(window_bounds.origin.x, 55, window_bounds.size.w, time_font_height));
  set_font_style(s_time_layer, s_time_font, GTextAlignmentCenter, GColorClear, GColorWhite);

  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(window_bounds.origin.x, 99, window_bounds.size.w, date_font_height));
  set_font_style(s_date_layer, s_date_font, GTextAlignmentCenter, GColorClear, GColorWhite);

  // Create Location layer
  s_location_layer = text_layer_create(GRect(window_bounds.origin.x, 119, window_bounds.size.w, date_font_height));
  set_font_style(s_location_layer, s_date_font, GTextAlignmentCenter, GColorClear, GColorWhite);

  // Create Pebble battery layer
  s_pebble_battery_layer = text_layer_create(GRect(window_bounds.origin.x, 152, 0, 16));
  set_font_style(s_pebble_battery_layer, s_battery_font, GTextAlignmentRight, GColorClear, GColorWhite);

  // Create pebble battery visual layer
  s_pebble_battery_visual_layer = layer_create(window_bounds);
  layer_set_update_proc(s_pebble_battery_visual_layer, battery_update_proc);

  // Create inversion layer
  s_inversion_layer = inverter_layer_create(window_bounds);
  //Check for saved option
  if(persist_read_bool(KEY_INVERT) == false) {
    layer_set_hidden((Layer*)s_inversion_layer, true);
  }

  // Build window layers
  layer_add_child(window_layer, s_background_layer);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_conditions_icon_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_conditions_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_location_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_pebble_battery_layer));
  layer_add_child(window_layer, s_pebble_battery_visual_layer);
  layer_add_child(window_layer, (Layer*)s_inversion_layer);

  battery_state = battery_state_service_peek();
  update_time();
  update_battery();
}

static void main_window_unload(Window *window) {
  // Destroy GBitmaps
  gbitmap_destroy(s_conditions_day_clear_bitmap);
  gbitmap_destroy(s_conditions_night_clear_bitmap);
  gbitmap_destroy(s_conditions_day_few_clouds_bitmap);
  gbitmap_destroy(s_conditions_night_few_clouds_bitmap);
  gbitmap_destroy(s_conditions_clouds_bitmap);
  gbitmap_destroy(s_conditions_rain_bitmap);
  gbitmap_destroy(s_conditions_thunderstorm_bitmap);
  gbitmap_destroy(s_conditions_snow_bitmap);
  gbitmap_destroy(s_conditions_fog_bitmap);

  // Destroy BitmapLayers
  bitmap_layer_destroy(s_conditions_icon_layer);

  // Destroy TextLayers
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(s_conditions_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_location_layer);
  text_layer_destroy(s_pebble_battery_layer);

  // Destroy Drawing layers
  layer_destroy(s_background_layer);
  layer_destroy(s_pebble_battery_visual_layer);

  // Destroy inversion layer
  inverter_layer_destroy(s_inversion_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // store incoming information
  static char temperature_buffer[] = "100°";
  static char conditions_buffer[64];
  static char location_buffer[64];
  static GBitmap *icon;

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_TEMPERATURE:
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%i%s", (int)t->value->int32, "°");
        break;
      case KEY_CONDITIONS_ICON:
        if(strcmp(t->value->cstring, "01d") == 0) {
          icon = s_conditions_day_clear_bitmap; // Day Clear
        } else if(strcmp(t->value->cstring, "01n") == 0) {
          icon = s_conditions_night_clear_bitmap; // Night Clear
        } else if(strcmp(t->value->cstring, "02d") == 0) {
          icon =  s_conditions_day_few_clouds_bitmap; // Few Clouds Day
        } else if(strcmp(t->value->cstring, "02n") == 0) {
          icon =  s_conditions_night_few_clouds_bitmap; // Few Clouds Night
        } else if(strcmp(t->value->cstring, "03d") == 0 ||
                  strcmp(t->value->cstring, "03n") == 0 ||
                  strcmp(t->value->cstring, "04d") == 0 ||
                  strcmp(t->value->cstring, "04n") == 0) {
          icon = s_conditions_clouds_bitmap; // Clouds Day/Night
        } else if(strcmp(t->value->cstring, "09d") == 0 ||
                  strcmp(t->value->cstring, "09n") == 0 ||
                  strcmp(t->value->cstring, "10d") == 0 ||
                  strcmp(t->value->cstring, "10n") == 0) {
          icon = s_conditions_rain_bitmap; // Rain Day/Night
        } else if(strcmp(t->value->cstring, "11d") == 0 ||
                  strcmp(t->value->cstring, "11n") == 0) {
          icon = s_conditions_thunderstorm_bitmap; // Thunderstorm Day/Night
        } else if(strcmp(t->value->cstring, "13d") == 0 ||
                  strcmp(t->value->cstring, "13n") == 0) {
          icon = s_conditions_snow_bitmap; // Snow Day/Night
        } else if(strcmp(t->value->cstring, "50d") == 0 ||
                  strcmp(t->value->cstring, "50n") == 0) {
          icon = s_conditions_fog_bitmap; // Mist Day/Night
        } else {
          icon = NULL; // Unknown
        }
        break;
      case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        break;
      case KEY_LOCATION:
        snprintf(location_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        break;
      case KEY_INVERT:
        if(strcmp(t->value->cstring, "yes") == 0) {
          //Set and save as inverted
          layer_set_hidden((Layer*)s_inversion_layer, false);
          persist_write_bool(KEY_INVERT, true);
        } else if(strcmp(t->value->cstring, "no") == 0) {
          //Set and save as not inverted
          layer_set_hidden((Layer*)s_inversion_layer, true);
          persist_write_bool(KEY_INVERT, false);
        }
        break;
      case KEY_VIBRATE:
        if(strcmp(t->value->cstring, "yes") == 0) {
          persist_write_bool(KEY_VIBRATE, true);
        } else if(strcmp(t->value->cstring, "no") == 0) {
          persist_write_bool(KEY_VIBRATE, false);
        }
        break;
      case KEY_UPDATE:
        persist_write_int(KEY_UPDATE, (int)t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_INFO, "Update Frequency: %d minutes", (int)persist_read_int(KEY_UPDATE));
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }
    // Look for next item
    t = dict_read_next(iterator);
  }

  bitmap_layer_set_bitmap(s_conditions_icon_layer, icon);
  text_layer_set_text(s_temperature_layer, temperature_buffer);
  text_layer_set_text(s_conditions_layer, conditions_buffer);
  text_layer_set_text(s_location_layer, location_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register with BatteryStateService
  battery_state_service_subscribe(pebble_battery_handler);

  // Register with BluetoothConnectionService
  bluetooth_connection_service_subscribe(bluetooth_connection_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
}

static void deinit() {
  // Destroy Window
   window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
