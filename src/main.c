#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
  
static Window *s_main_window;
static TextLayer *s_hour_layer;
static TextLayer *s_time_seperator_layer;
static TextLayer *s_minute_layer;
static TextLayer *s_date_layer;
static TextLayer *s_temperature_layer;
static GFont s_hour_font;
static GFont s_minute_font;
static GFont s_date_font;
static GFont s_temperature_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Create a long-lived buffer
  static char buffer[] = "00";
  static char buffer2[] = "00";
  static char buffer3[] = "Wed, September 30";
  
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00"), "%H", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00"), "%I", tick_time);
  }
  
  strftime(buffer2, sizeof("00"), "%M", tick_time);
  strftime(buffer3, sizeof("Wed, September 30"), "%a, %B %e", tick_time);
  
  // Display the time on the TextLayer
  text_layer_set_text(s_hour_layer, buffer);
  text_layer_set_text(s_minute_layer, buffer2);
  text_layer_set_text(s_date_layer, buffer3);
  
}

static void main_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create time TextLayer
  s_hour_layer = text_layer_create(GRect(0, 40, 58, 56));
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorWhite);
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  text_layer_set_text(s_hour_layer, "00");
  s_hour_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INCONSOLATA_BOLD_54));
  text_layer_set_font(s_hour_layer, s_hour_font);
  
  s_time_seperator_layer = text_layer_create(GRect(58, 35, 28, 56));
  text_layer_set_background_color(s_time_seperator_layer, GColorClear);
  text_layer_set_text_color(s_time_seperator_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_seperator_layer, GTextAlignmentCenter);
  text_layer_set_text(s_time_seperator_layer, ":");
  text_layer_set_font(s_time_seperator_layer, s_hour_font);
  
  s_minute_layer = text_layer_create(GRect(86, 40, 58, 56));
  text_layer_set_background_color(s_minute_layer, GColorClear);
  text_layer_set_text_color(s_minute_layer, GColorWhite);
  text_layer_set_text_alignment(s_minute_layer, GTextAlignmentCenter);
  text_layer_set_text(s_minute_layer, "00");
  s_minute_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INCONSOLATA_54));
  text_layer_set_font(s_minute_layer, s_minute_font);
  
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 96, 144, 18));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "Wed, September 30");
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INCONSOLATA_16));
  text_layer_set_font(s_date_layer, s_date_font);
  
  // Create temperature layer
  s_temperature_layer = text_layer_create(GRect(48, 0, 96, 56));
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentRight);
  text_layer_set_text(s_temperature_layer, "100°");
  s_temperature_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INCONSOLATA_48));
  text_layer_set_font(s_temperature_layer, s_temperature_font);
  
  
  // Build window layers
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hour_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_seperator_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_minute_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temperature_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  // Unload GFonts
  fonts_unload_custom_font(s_hour_font);
  fonts_unload_custom_font(s_minute_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_temperature_font);
  
  // Destroy TextLayers
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_time_seperator_layer);
  text_layer_destroy(s_minute_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_temperature_layer);
  
  // Destroy GBitmaps
  gbitmap_destroy(s_background_bitmap);
  
  // Destroy BitmapLayers
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
  
  // Read first item
  Tuple *t = dict_read_first(iterator);
  
  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_TEMPERATURE:
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%i%s", (int)t->value->int32, "°");
        break;
      case KEY_CONDITIONS:
        // snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }
    
    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  // snprintf(weather_layer_buffer, sizeof(temperature_buffer), "%s", temperature_buffer);
  text_layer_set_text(s_temperature_layer, temperature_buffer);
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
