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
// ---- Layer + dynamic macro lighting (RGB Matrix) ----
// Priority: recording > playback flash > layer colour > normal Vial lighting
// Hues (0-255): red 0, orange 21, yellow 43, green 85, cyan 128, blue 170, purple 191, pink 234

static const uint8_t layer_hues[] = {
    [1] = 170,  // Layer 1: blue
    [2] = 191,  // Layer 2: purple
    [3] = 43,   // Layer 3: yellow
};

static bool     dm_recording   = false;
static int8_t   dm_direction   = 0;
static bool     dm_flashing    = false;
static uint16_t dm_flash_timer = 0;

// Solid colour at the current brightness, not saved to EEPROM
static void show_hue(uint8_t hue) {
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_sethsv_noeeprom(hue, 255, rgb_matrix_get_val());
}

static void update_lighting(layer_state_t state) {
    uint8_t layer = get_highest_layer(state);

    if (dm_recording) {
        show_hue(dm_direction > 0 ? 0 : 21);   // red = macro 1, orange = macro 2
    } else if (dm_flashing) {
        show_hue(85);                          // green flash on playback
    } else if (layer > 0 && layer < ARRAY_SIZE(layer_hues)) {
        show_hue(layer_hues[layer]);
    } else {
        rgb_matrix_reload_from_eeprom();       // base layer: your normal Vial lighting
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    update_lighting(state);
    return state;
}

bool dynamic_macro_record_start_user(int8_t direction) {
    dm_recording = true;
    dm_direction = direction;
    dm_flashing  = false;
    update_lighting(layer_state);
    return true;
}

bool dynamic_macro_record_end_user(int8_t direction) {
    dm_recording = false;
    update_lighting(layer_state);
    return true;
}

bool dynamic_macro_play_user(int8_t direction) {
    dm_flashing    = true;
    dm_flash_timer = timer_read();
    update_lighting(layer_state);
    return true;
}

void housekeeping_task_user(void) {
    if (dm_flashing && timer_elapsed(dm_flash_timer) > 400) {
        dm_flashing = false;
        update_lighting(layer_state);
    }
}
