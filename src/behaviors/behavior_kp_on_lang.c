/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_kp_on_lang

#include <stdlib.h>
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/keymap.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/language.h>
#include <zmk/events/keycode_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_kp_on_lang_config {
    struct zmk_behavior_binding behavior_ru;  // клавиша для переключения на русский
    struct zmk_behavior_binding behavior_en;  // клавиша для переключения на английский
    uint8_t layer_en; // слой EN
    uint8_t layer_ru; // слой RU
    uint8_t language; // целевой язык для нажатия (слой_en - английский, слой_ru - русский)
};

struct behavior_kp_on_lang_data {};

static int behavior_kp_on_lang_init(const struct device *dev) { return 0; };

// struct zmk_behavior_binding get_switch_back_binding(uint8_t target_lang,
//                                                     const struct zmk_behavior_binding *original) {
    // struct zmk_behavior_binding copy;
    // memcpy(&copy, original, sizeof(struct zmk_behavior_binding));
    // copy.param1 = target_lang;
    // return copy;
// }

static int kp_on_lang_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                             struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_kp_on_lang_config *config = dev->config;
    // struct behavior_kp_on_lang_data *data = dev->data;

    // 1. переключаем язык клавиатуры, если он не совпадает с целевым
    if (get_os_language() != config->language) {

        // ### Нажимаем клавишу переключения в зависимости от выбранного языка
        if (config->language == config->layer_en) {
            // Английский язык
            // Помещаем в очередь нажатие И отпускание 
            zmk_behavior_queue_add(&event, config->behavior_en, true, 0);
            zmk_behavior_queue_add(&event, config->behavior_en, false, 0);
            LOG_DBG("Switched to English language, layer %d", config->layer_en);
        } 
        else {
            // Русский язык
            // Помещаем в очередь нажатие И отпускание 
            zmk_behavior_queue_add(&event, config->behavior_ru, true, 0);
            zmk_behavior_queue_add(&event, config->behavior_ru, false, 0);
            LOG_DBG("Switched to Russian language, layer %d", config->layer_ru);
        }
        set_os_language(config->language);
        // ###

        // 2. Возвращаем нажатие самой клавиши с задержкой, чтобы сработало переключения языков
        return raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp + 60);
    }

    // 2. Возвращаем нажатие самой клавиши сразу
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp);
}

    // const uint8_t current_lang = zmk_language_state();
    // if (current_lang != binding->param2) {
    //     LOG_DBG("KP_LANG switch from %d to %d", current_lang, config->switch_behavior.param1);
    //     data->switch_back_behavior =
    //         get_switch_back_binding(current_lang, &config->switch_behavior);
    //     data->switch_back = true;
    //     LOG_DBG("KP_LANG switchback %d", data->switch_back_behavior.param1);
    //     zmk_behavior_queue_add(&event, config->switch_behavior, true, 0);
    //     zmk_behavior_queue_add(&event, config->switch_behavior, false, 0);
    // }
    // return raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp);
//}

static int kp_on_lang_keymap_binding_released(struct zmk_behavior_binding *binding,
                                              struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_kp_on_lang_config *config = dev->config;
    // struct behavior_kp_on_lang_data *data = dev->data;

    // 1. переключаем язык клавиатуры назад, если нужно
    if (get_os_language() != get_kb_language()) {

        // ### Нажимаем клавишу переключения в зависимости от выбранного языка
        if (get_kb_language() == config->layer_en) {
            // Английский язык
            // Помещаем в очередь нажатие И отпускание 
            zmk_behavior_queue_add(&event, config->behavior_en, true, 0);
            zmk_behavior_queue_add(&event, config->behavior_en, false, 0);
            LOG_DBG("Switched to English language, layer %d", config->layer_en);
        } 
        else {
            // Русский язык
            // Помещаем в очередь нажатие И отпускание 
            zmk_behavior_queue_add(&event, config->behavior_ru, true, 0);
            zmk_behavior_queue_add(&event, config->behavior_ru, false, 0);
            LOG_DBG("Switched to Russian language, layer %d", config->layer_ru);
        }
        set_os_language(get_kb_language());
        // ###

        // 2. Возвращаем отпускание самой клавиши с задержкой, чтобы сработало переключения языков
        return raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp + 60);
    }

    // 2. Возвращаем отпускание самой клавиши
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp);

}

    // if (data->switch_back) {
    //     zmk_behavior_queue_add(&event, data->switch_back_behavior, true, 0);
    //     zmk_behavior_queue_add(&event, data->switch_back_behavior, false, 0);
    //     data->switch_back = false;
    // }
    // return raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp);

static const struct behavior_driver_api behavior_kp_on_lang_driver_api = {
    .binding_pressed = kp_on_lang_keymap_binding_pressed,
    .binding_released = kp_on_lang_keymap_binding_released};

#define KP_ON_LANG_INST(n)                                                                         \
    static struct behavior_kp_on_lang_data behavior_kp_on_lang_data_##n = {};                      \
    static struct behavior_kp_on_lang_config behavior_kp_on_lang_config_##n = {                    \
        .behavior_en = ZMK_KEYMAP_EXTRACT_BINDING(0, DT_DRV_INST(n)),                              \
        .behavior_ru = ZMK_KEYMAP_EXTRACT_BINDING(1, DT_DRV_INST(n)),                              \
        .layer_en = DT_INST_PROP(n, en_layer),                                                     \
        .layer_ru = DT_INST_PROP(n, ru_layer),                                                     \
        .language = DT_INST_PROP(n, language),                                                     \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_kp_on_lang_init, NULL, &behavior_kp_on_lang_data_##n,      \
                            &behavior_kp_on_lang_config_##n, APPLICATION,                          \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_kp_on_lang_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_ON_LANG_INST)
