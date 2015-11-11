#include <pebble.h>

#define KEY_WORD 0
#define KEY_DEFINITION 1
#define KEY_REQUEST_TYPE 2
#define KEY_USER_ID 3
#define KEY_RESEND_REQUEST 4

static Window *s_main_window;
static Window *s_definition_window;
static TextLayer *s_time_layer;
static TextLayer *s_flash_layer;
static TextLayer *s_word_layer;
static TextLayer *s_definition_layer;
static ActionBarLayer *action_bar;
static Layer *indicator;
static GFont s_flash_font;
static GBitmap *s_up_bitmap;
static GBitmap *s_down_bitmap;


static bool has_word;
static bool signed_in;
static char word_buffer[20];
static char user_id[20];
static char definition_buffer[200];

static bool requested_word;
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Do something
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, KEY_REQUEST_TYPE, 2);

  // Send the message!
  app_message_outbox_send();
  
  has_word = false;
  window_stack_pop(true);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Do something
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, KEY_REQUEST_TYPE, 1);

  // Send the message!
  app_message_outbox_send();
  
  has_word = false;
  window_stack_pop(true);
}

void definition_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_click_handler);
}

static void definition_window_load(Window * window) {
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_word_layer = text_layer_create(
      GRect(0, 0, bounds.size.w, 30));
  
  // Style the text
  text_layer_set_background_color(s_word_layer, GColorBlack);
  text_layer_set_text_color(s_word_layer, GColorClear);
  
  text_layer_set_text(s_word_layer, word_buffer);
  text_layer_set_font(s_word_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_word_layer, GTextAlignmentCenter);
  layer_add_child(window_layer,text_layer_get_layer(s_word_layer));
  
  s_definition_layer = text_layer_create(
      GRect(2,30,bounds.size.w-4-ACTION_BAR_WIDTH,bounds.size.h-30));
  text_layer_set_text_color(s_definition_layer, GColorBlack);
  text_layer_set_background_color(s_definition_layer, GColorClear);
  text_layer_set_text(s_definition_layer, definition_buffer);
  text_layer_set_font(s_definition_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer,text_layer_get_layer(s_definition_layer));
  
  // Create the action bar
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  // Set the click config provider:
  action_bar_layer_set_click_config_provider(action_bar,
                                             definition_click_config_provider);

  s_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_UP);
  s_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DOWN);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, s_up_bitmap);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, s_down_bitmap);
}

static void definition_window_unload(Window * window) {
    text_layer_destroy(s_word_layer);
    text_layer_destroy(s_definition_layer);
    gbitmap_destroy(s_up_bitmap);
    gbitmap_destroy(s_down_bitmap);
    action_bar_layer_destroy(action_bar);
}

static void send_int(int key, int value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int(iter, key, &value, sizeof(int), true);
  app_message_outbox_send();
}

static void draw_indicator_proc(Layer *this_layer, GContext *ctx) {
  // Draw things here using ctx
  if (has_word) {
    GRect bounds = layer_get_bounds(this_layer);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, GPoint(bounds.size.w/2,bounds.size.h/2), bounds.size.w/2);
  }
}


static void sendPhoneAuthentication() {
  // Check if the user id is set
  //APP_LOG(APP_LOG_LEVEL_INFO, "Already signed in !");
  APP_LOG(APP_LOG_LEVEL_INFO, user_id);
      
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_cstring(iter, KEY_USER_ID, user_id);
  // Send the message!
  app_message_outbox_send();
  signed_in = true;
}


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (signed_in) { 
    // If we have a word, get its definition
    if (has_word == true) {
      window_stack_push(s_definition_window, true);
    } else {
      // Otherwise get one
      APP_LOG(APP_LOG_LEVEL_INFO, "Requesting word");
      send_int(KEY_REQUEST_TYPE,0);
      // Really this just means we have requested a word
      requested_word = true;
    }
  }
}

static void main_click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}
                      
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, KEY_REQUEST_TYPE, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void main_window_load(Window * window) {  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 50), bounds.size.w, 50));
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Create the new word indicator
  indicator = layer_create(GRect(bounds.size.w-PBL_IF_ROUND_ELSE(20, 10),bounds.size.h/2-5,7,7));
  layer_set_update_proc(indicator, draw_indicator_proc);
  layer_add_child(window_layer,indicator);
  
  // Create the FlashView
  s_flash_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(125, 110), bounds.size.w, 25));
  
  // Style it
  text_layer_set_background_color(s_flash_layer, GColorClear);
  text_layer_set_text_color(s_flash_layer, GColorBlack);
  text_layer_set_text_alignment(s_flash_layer, GTextAlignmentCenter);
  text_layer_set_text(s_flash_layer, "");

  // Create second custom font, apply it and add to Window
  //s_flash_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
  
  text_layer_set_font(s_flash_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_flash_layer));
}

                      
static void main_window_unload(Window * window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_flash_layer);
  layer_destroy(indicator);
  fonts_unload_custom_font(s_flash_font);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read tuples for data
  Tuple *word_tuple = dict_find(iterator, KEY_WORD);
  Tuple *definition_tuple = dict_find(iterator, KEY_DEFINITION);
  Tuple *user_id_tuple = dict_find(iterator, KEY_USER_ID);
  Tuple *resend_request_tuple = dict_find(iterator, KEY_RESEND_REQUEST);
  // If all data is available, use it
  if(word_tuple && definition_tuple && !has_word) {
    // Check if there's a new word
    if (strcmp(word_tuple->value->cstring, "") != 0) {
      snprintf(word_buffer, sizeof(word_buffer), "%s", word_tuple->value->cstring);
      snprintf(definition_buffer, sizeof(definition_buffer), "%s", definition_tuple->value->cstring);
      text_layer_set_text(s_flash_layer, word_buffer);
      has_word = true;
      requested_word = false;
      APP_LOG(APP_LOG_LEVEL_INFO, "Requesting word");
      // Vibrate!
      //vibes_short_pulse();
    }
  }
  else if (user_id_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Recieved signin credentials!");
    APP_LOG(APP_LOG_LEVEL_INFO, user_id_tuple->value->cstring);
    if (strcmp(user_id_tuple->value->cstring, "") != 0) {
       signed_in = true;
       snprintf(user_id, sizeof(user_id), "%s", user_id_tuple->value->cstring);
    }
  } else if (resend_request_tuple) {
    if (signed_in) {
      sendPhoneAuthentication();
    }
  }
  //just_vibrated = false;
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
  // Assume no word to begin
  has_word = false;
  signed_in = false;
  
 // APP_LOG(APP_LOG_LEVEL_INFO, "HELLO!");
  
  if (persist_exists(KEY_USER_ID)) {
    persist_read_string(KEY_USER_ID, user_id, sizeof(user_id));
    if (strcmp(user_id, "") != 0){
      app_timer_register(300, sendPhoneAuthentication, NULL);
    }
  }
  
  // Gets a window object
  s_main_window = window_create();
  s_definition_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
    
  window_set_window_handlers(s_definition_window, (WindowHandlers) {
    .load = definition_window_load,
    .unload = definition_window_unload
  });
  
  // Display the window, animated=true
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register Callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
      
  // Register Input callbacks
  window_set_click_config_provider(s_main_window, main_click_config_provider);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  if (signed_in){
    persist_write_string(KEY_USER_ID, user_id);
  }
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
