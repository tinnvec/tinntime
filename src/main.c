#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS_ICON 1
#define KEY_CONDITIONS 2
#define KEY_LOCATION 3
  
static Window *s_main_window;
static BitmapLayer *s_background_layer;
static BitmapLayer *s_conditions_icon_layer;
static TextLayer *s_temperature_layer;
static TextLayer *s_conditions_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_location_layer;
static GFont s_temperature_font;
static GFont s_time_font;
static GFont s_date_font;
static GBitmap *s_background_bitmap;
static GBitmap *s_conditions_day_clear_bitmap;
static GBitmap *s_conditions_night_clear_bitmap;
static GBitmap *s_conditions_day_few_clouds_bitmap;
static GBitmap *s_conditions_night_few_clouds_bitmap;
static GBitmap *s_conditions_clouds_bitmap;
static GBitmap *s_conditions_rain_bitmap;
static GBitmap *s_conditions_thunderstorm_bitmap;
static GBitmap *s_conditions_snow_bitmap;
static GBitmap *s_conditions_fog_bitmap;
static GBitmap *s_conditions_unknown_bitmap;

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

static void main_window_load(Window *window) {
  // Create GFonts
  s_temperature_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INCONSOLATA_48));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INCONSOLATA_BOLD_54));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INCONSOLATA_16));
  
  // Create GBitmaps
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_conditions_day_clear_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_CLEAR);
  s_conditions_night_clear_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NIGHT_CLEAR);
  s_conditions_day_few_clouds_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DAY_FEW_CLOUDS);
  s_conditions_night_few_clouds_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NIGHT_FEW_CLOUDS);
  s_conditions_clouds_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOUDS);
  s_conditions_rain_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RAIN);
  s_conditions_thunderstorm_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_THUNDERSTORM);
  s_conditions_snow_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SNOW);
  s_conditions_fog_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOG);
  s_conditions_unknown_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_UNKNOWN);
  
  // Create Background Layer
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  
  // Create conditions icon Layer
  s_conditions_icon_layer = bitmap_layer_create(GRect(0, 1, 60, 56));
  bitmap_layer_set_alignment(s_conditions_icon_layer, GAlignTop);
  bitmap_layer_set_bitmap(s_conditions_icon_layer, s_conditions_unknown_bitmap);
  
  // Create temperature layer
  s_temperature_layer = text_layer_create(GRect(48, -8, 96, 56));
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentRight);
  text_layer_set_text(s_temperature_layer, "--°");
  text_layer_set_font(s_temperature_layer, s_temperature_font);
  
  // Create conditions layer
  s_conditions_layer = text_layer_create(GRect(0, 42, 144, 20));
  text_layer_set_background_color(s_conditions_layer, GColorClear);
  text_layer_set_text_color(s_conditions_layer, GColorWhite);
  text_layer_set_text_alignment(s_conditions_layer, GTextAlignmentCenter);
  text_layer_set_text(s_conditions_layer, "Loading...");
  text_layer_set_font(s_conditions_layer, s_date_font);
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 45, 144, 56));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text(s_time_layer, "--:--");
  text_layer_set_font(s_time_layer, s_time_font);
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 101, 144, 20));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "Wed, September 30");
  text_layer_set_font(s_date_layer, s_date_font);
  
  // Create Location layer
  s_location_layer = text_layer_create(GRect(0, 119, 144, 20));
  text_layer_set_background_color(s_location_layer, GColorClear);
  text_layer_set_text_color(s_location_layer, GColorWhite);
  text_layer_set_text_alignment(s_location_layer, GTextAlignmentCenter);
  text_layer_set_text(s_location_layer, "Loading...");
  text_layer_set_font(s_location_layer, s_date_font);
  
  // Build window layers
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_conditions_icon_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temperature_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_conditions_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_location_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  // Unload GFonts
  fonts_unload_custom_font(s_temperature_font);
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  
  // Destroy TextLayers
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(s_conditions_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_location_layer);
  
  // Destroy GBitmaps
  gbitmap_destroy(s_background_bitmap);
  gbitmap_destroy(s_conditions_day_clear_bitmap);
  gbitmap_destroy(s_conditions_night_clear_bitmap);
  gbitmap_destroy(s_conditions_day_few_clouds_bitmap);
  gbitmap_destroy(s_conditions_night_few_clouds_bitmap);
  gbitmap_destroy(s_conditions_clouds_bitmap);
  gbitmap_destroy(s_conditions_rain_bitmap);
  gbitmap_destroy(s_conditions_thunderstorm_bitmap);
  gbitmap_destroy(s_conditions_snow_bitmap);
  gbitmap_destroy(s_conditions_fog_bitmap);
  gbitmap_destroy(s_conditions_unknown_bitmap);
  
  // Destroy BitmapLayers
  bitmap_layer_destroy(s_conditions_icon_layer);
  bitmap_layer_destroy(s_background_layer);  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
    
    // send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // store incoming information
  static char temperature_buffer[8];
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
        if (strcmp(t->value->cstring, "01d") == 0) {
          icon = s_conditions_day_clear_bitmap; // Day Clear
        } else if (strcmp(t->value->cstring, "01n") == 0) {
          icon = s_conditions_night_clear_bitmap; // Night Clear
        } else if (strcmp(t->value->cstring, "02d") == 0) {
          icon =  s_conditions_day_few_clouds_bitmap; // Few Clouds Day
        } else if (strcmp(t->value->cstring, "02n") == 0) {
          icon =  s_conditions_night_few_clouds_bitmap; // Few Clouds Night
        } else if (strcmp(t->value->cstring, "03d") == 0 ||
                   strcmp(t->value->cstring, "03n") == 0 ||
                   strcmp(t->value->cstring, "04d") == 0 ||
                   strcmp(t->value->cstring, "04n") == 0) {
          icon = s_conditions_clouds_bitmap; // Clouds Day/Night
        } else if (strcmp(t->value->cstring, "09d") == 0 ||
                   strcmp(t->value->cstring, "09n") == 0 ||
                   strcmp(t->value->cstring, "10d") == 0 ||
                   strcmp(t->value->cstring, "10n") == 0) {
          icon = s_conditions_rain_bitmap; // Rain Day/Night
        } else if (strcmp(t->value->cstring, "11d") == 0 ||
                   strcmp(t->value->cstring, "11n") == 0) {
          icon = s_conditions_thunderstorm_bitmap; // Thunderstorm Day/Night
        } else if (strcmp(t->value->cstring, "13d") == 0 ||
                   strcmp(t->value->cstring, "13n") == 0) {
          icon = s_conditions_snow_bitmap; // Snow Day/Night
        } else if (strcmp(t->value->cstring, "50d") == 0 ||
                   strcmp(t->value->cstring, "50n") == 0) {
          icon = s_conditions_fog_bitmap; // Mist Day/Night
        } else {
          icon = s_conditions_unknown_bitmap; // Unknown
        }
        break;
      case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        break;
      case KEY_LOCATION:
        snprintf(location_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }
    
    // Look for next item
    t = dict_read_next(iterator);
  }
  
  bitmap_layer_set_bitmap(s_conditions_icon_layer, icon);
  
  // Assemble full string and display
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
