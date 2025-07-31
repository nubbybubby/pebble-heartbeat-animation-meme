#include <pebble.h>

static Window          *main_window;

static GBitmap         *animation_bitmap;
static BitmapLayer     *animation_layer;
static GBitmapSequence *animation_sequence;

static TextLayer       *subtitle_layer;
static TextLayer       *subtitle_layer_center;

uint8_t scene;
bool    repeat;

static const uint32_t scenes[6] = {
    RESOURCE_ID_CHAR1_PART1, 
    RESOURCE_ID_CHAR1_PART2, 
    RESOURCE_ID_CHAR1_PART3,
    RESOURCE_ID_CHAR2_PART1,
    RESOURCE_ID_CHAR2_PART2,
    RESOURCE_ID_CHAR2_PART3,
};

bool subtitle_toggle;

static const char *subtitles[6] = {
    "I WANTED YOU TO KNOW",
    "THAT I AM READY TO GO",
    "HEARTBEAT, MY HEARTBEAT",

    "I WANTED YOU TO KNOW",
    "WHENEVER YOU ARE AROUND",
    "CAN'T SPEAK, I CAN'T SPEAK"
};

static void load_scene();

static void timer_handler(void *context) {
    uint32_t next_delay;

    if (gbitmap_sequence_update_bitmap_next_frame(animation_sequence, animation_bitmap, &next_delay)) {
        bitmap_layer_set_bitmap(animation_layer, animation_bitmap);
        layer_mark_dirty(bitmap_layer_get_layer(animation_layer));
        app_timer_register(101, timer_handler, NULL);
    } else {
        if (scene >= 5 && repeat == false) {
            scene = 0;
            repeat = true;
        } else if (scene >= 5 && repeat == true) {
            // This crashes the app instead of closing it normally. Whatever, same thing.
            bitmap_layer_destroy(animation_layer);
            gbitmap_destroy(animation_bitmap);
            vibes_cancel();
        } else {
            scene++;
        }
        load_scene();
    }
}

static void subtitle_timer_handler() {
    if (subtitle_toggle == false) {
        if (scene == 2 || scene == 4 || scene == 5) {
            layer_set_hidden(text_layer_get_layer(subtitle_layer_center), true);
            layer_set_hidden(text_layer_get_layer(subtitle_layer), false);
        } else {
            layer_set_hidden(text_layer_get_layer(subtitle_layer_center), false);
            layer_set_hidden(text_layer_get_layer(subtitle_layer), true);
        }
    } else {
        layer_set_hidden(text_layer_get_layer(subtitle_layer_center), true);
        layer_set_hidden(text_layer_get_layer(subtitle_layer), true);
    }
    text_layer_set_text(subtitle_layer, subtitles[scene]);
    text_layer_set_text(subtitle_layer_center, subtitles[scene]);
}

static const uint32_t const beat_pattern[] = { 45, 480, 80 };

VibePattern beat = {
    .durations = beat_pattern,
    .num_segments = ARRAY_LENGTH(beat_pattern),
};

static void beat_handler() {
    vibes_enqueue_custom_pattern(beat);
    app_timer_register(480, beat_handler, NULL);
}

static void load_scene() {
    if (animation_sequence) {
        gbitmap_sequence_destroy(animation_sequence);
        animation_sequence = NULL;
    }
    if (animation_bitmap) {
        gbitmap_destroy(animation_bitmap);
        animation_bitmap = NULL;
    }

    animation_sequence = gbitmap_sequence_create_with_resource(scenes[scene]);

    GSize image_frame_size = gbitmap_sequence_get_bitmap_size(animation_sequence);
    animation_bitmap = gbitmap_create_blank(image_frame_size, GBitmapFormat8Bit);

    if (scene == 0 && repeat == false) {
        app_timer_register(750, timer_handler, NULL);
        app_timer_register(750, beat_handler, NULL);
        #if PBL_RECT
            text_layer_set_text(subtitle_layer_center, "'CAUSE");
            app_timer_register(815, subtitle_timer_handler, NULL);
        #endif
    } else {
        app_timer_register(0, timer_handler, NULL);
        #if PBL_RECT
            app_timer_register(150, subtitle_timer_handler, NULL);
        #endif
    }
}

static void heartbeat_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    #if PBL_RECT
        subtitle_layer = text_layer_create(GRect(0, 136, window_bounds.size.w, 30));
        subtitle_layer_center = text_layer_create(GRect(0, 143, window_bounds.size.w, 30));
        
        text_layer_set_text_color(subtitle_layer, GColorWhite);
        text_layer_set_text_color(subtitle_layer_center, GColorWhite);
        
        text_layer_set_text_alignment(subtitle_layer, GTextAlignmentCenter);
        text_layer_set_text_alignment(subtitle_layer_center, GTextAlignmentCenter);
        
        text_layer_set_background_color(subtitle_layer, GColorClear);
        text_layer_set_background_color(subtitle_layer_center, GColorClear);

        layer_add_child(window_layer, text_layer_get_layer(subtitle_layer));
        layer_add_child(window_layer, text_layer_get_layer(subtitle_layer_center));
    #endif

    window_set_background_color(window, GColorBlack);    
    
    animation_layer = bitmap_layer_create(window_bounds);

    layer_add_child(window_layer, bitmap_layer_get_layer(animation_layer));
    
    load_scene();
}

static void heartbeat_unload(Window *window) {
    vibes_cancel();
    text_layer_destroy(subtitle_layer);
    text_layer_destroy(subtitle_layer_center);
    gbitmap_sequence_destroy(animation_sequence);
    gbitmap_destroy(animation_bitmap);
    bitmap_layer_destroy(animation_layer);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (!subtitle_toggle) {
        subtitle_toggle = true;
        subtitle_timer_handler();
        return;
    }
    subtitle_toggle = false;
    subtitle_timer_handler();
}

static void click_config_provider(void *context) {
    ButtonId id = BUTTON_ID_SELECT;
    window_single_click_subscribe(id, select_click_handler);
}

static void init() {
    main_window = window_create();
    
    #if PBL_RECT
        window_set_click_config_provider(main_window, click_config_provider);
    #endif

    window_set_window_handlers(main_window, (WindowHandlers) {
        .load = heartbeat_load,
        .unload = heartbeat_unload
    });
    
    window_stack_push(main_window, false);
    light_enable(true);
}

static void deinit() {
    window_destroy(main_window);
    light_enable(false);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
