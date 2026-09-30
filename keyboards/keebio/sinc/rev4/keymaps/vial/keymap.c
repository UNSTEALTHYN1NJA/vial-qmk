// Copyright 2023 Danny Nguyen (danny@keeb.io)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT_all(
    KC_MUTE,          KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,  KC_INS,
    KC_F1,   KC_F2,   KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_DEL,  KC_BSPC, KC_HOME,
    KC_F3,   KC_F4,   KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_END,
    KC_F5,   KC_F6,   KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NUHS, KC_ENT,  KC_PGUP,
    KC_F7,   KC_F8,   KC_LSFT, KC_NUBS, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_UP,   KC_PGDN,
    KC_F9,   KC_F10,  KC_LCTL, KC_LALT, KC_LGUI, MO(1),   KC_SPC,  KC_SPC,           MO(1),   KC_SPC,  KC_RALT, KC_RCTL, KC_RGUI, KC_LEFT, KC_DOWN, KC_RGHT
  ),

  [1] = LAYOUT_all(
    _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    RGB_HUI, RGB_HUD, QK_GESC, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______, _______, _______,
    RGB_SAI, RGB_SAD, RGB_TOG, RGB_MOD, _______, KC_UP,   _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    RGB_VAI, RGB_VAD, _______, _______, KC_LEFT, KC_DOWN, KC_RGHT, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______, _______
  )
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = { ENCODER_CCW_CW(KC_VOLU, KC_VOLD), ENCODER_CCW_CW(KC_PGUP, KC_PGDN) },
    [1] = { ENCODER_CCW_CW(RGB_MOD, RGB_RMOD), ENCODER_CCW_CW(KC_MNXT, KC_MPRV) }
};
#endif


/////////////////////////////////////////////////////////////////////////////////////////
// Colour for each layer as {red, green, blue}, 0-255.
// Layer 0 is left empty so it keeps the normal Vial effect.
static const uint8_t layer_colours[][3] = {
    [1] = {0, 0, 255},    // Layer 1: blue
    [2] = {255, 0, 0},    // Layer 2: red
    [3] = {0, 255, 0},    // Layer 3: green
};

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);

    if (layer == 0 || layer >= ARRAY_SIZE(layer_colours)) {
        return false;  // base layer: leave the normal effect alone
    }

    // Scale by the current brightness so it follows your Vial brightness setting
    uint8_t v = rgb_matrix_get_val();
    uint8_t r = layer_colours[layer][0] * v / 255;
    uint8_t g = layer_colours[layer][1] * v / 255;
    uint8_t b = layer_colours[layer][2] * v / 255;

    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, r, g, b);
    }
    return false;
}




//////////////////////////////////////////////////////////////////////////////////////
// ---- Dynamic macro lighting ----
// Hue values (0-255): red = 0, orange = 21, yellow = 43, green = 85, blue = 170, purple = 191

static bool     dm_flashing    = false;
static uint16_t dm_flash_timer = 0;

// Switch to a solid colour at the current brightness (not saved to EEPROM)
static void dm_show_colour(uint8_t hue) {
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_sethsv_noeeprom(hue, 255, rgb_matrix_get_val());
}

bool dynamic_macro_record_start_user(int8_t direction) {
    dm_flashing = false;
    // direction is 1 for macro 1, -1 for macro 2
    dm_show_colour(direction > 0 ? 0 : 21);  // red for macro 1, orange for macro 2
    return true;
}

bool dynamic_macro_record_end_user(int8_t direction) {
    rgb_matrix_reload_from_eeprom();  // back to your normal Vial lighting
    return true;
}

bool dynamic_macro_play_user(int8_t direction) {
    dm_show_colour(85);  // green flash on playback
    dm_flashing    = true;
    dm_flash_timer = timer_read();
    return true;
}

void housekeeping_task_user(void) {
    // End the playback flash after 400 ms
    if (dm_flashing && timer_elapsed(dm_flash_timer) > 400) {
        dm_flashing = false;
        rgb_matrix_reload_from_eeprom();
    }
}
