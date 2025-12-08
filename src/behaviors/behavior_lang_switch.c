/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_lang_switch

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/keymap.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/language.h>

uint8_t current_language_state = 0;

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_lang_config {
    struct zmk_behavior_binding behavior_ru;  // переклюключение на русский
    struct zmk_behavior_binding behavior_en;  // переключение на английский
    uint8_t layer_en; // слой EN
    uint8_t layer_ru; // слой RU
};

struct behavior_lang_data {};

static int behavior_lang_init(const struct device *dev) { return 0; };


static int lang_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {

    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_lang_config *config = dev->config;

    uint8_t target_language;

    // Проверяем параметр (0 - английский, 1 - русский, любое другое значение - переключить на противоположный)
    if (binding->param1 == 0) {
        target_language = 0; // Английский
    } 
    else if (binding->param1 == 1) {
        target_language = 1; // Русский
    }
    else {
        // Любое другое значение - переключить на противоположный язык
        if (current_language_state == 0) {
            target_language = 1; // Переключить с английского на русский
        } else {
            target_language = 0; // Переключить с русского на английский
        }
    }
    
    // Если язык уже активен, ничего не делаем
    if (current_language_state == target_language) {
        LOG_DBG("Language %d is already active", target_language);
        return ZMK_BEHAVIOR_OPAQUE;
    }
    
    // Обновляем глобальную переменную
    current_language_state = target_language;
    
    // Переключаем слой в зависимости от выбранного языка
    if (target_language == 0) {
        // Английский язык
        zmk_keymap_layer_to(config->layer_en, false);
        // Помещаем в очередь нажатие И отпускание 
        zmk_behavior_queue_add(&event, config->behavior_en, true, 0);
        zmk_behavior_queue_add(&event, config->behavior_en, false, 0);
        LOG_DBG("Switched to English language, layer %d", config->layer_en);
    } 
    else {
        // Русский язык
        zmk_keymap_layer_to(config->layer_ru, false);
        // Помещаем в очередь нажатие И отпускание 
        zmk_behavior_queue_add(&event, config->behavior_ru, true, 0);
        zmk_behavior_queue_add(&event, config->behavior_ru, false, 0);
        LOG_DBG("Switched to Russian language, layer %d", config->layer_ru);
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int lang_keymap_binding_released(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_lang_driver_api = {
    .binding_pressed = lang_keymap_binding_pressed,
    .binding_released = lang_keymap_binding_released};

#define LANG_INST(n)                                                                               \
    static struct behavior_lang_data behavior_lang_data_##n = {};                                  \
    static struct behavior_lang_config behavior_lang_config_##n = {                                \
        .behavior_en = ZMK_KEYMAP_EXTRACT_BINDING(0, DT_DRV_INST(n)),                              \
        .behavior_ru = ZMK_KEYMAP_EXTRACT_BINDING(1, DT_DRV_INST(n)),                              \
        .layer_en = DT_INST_PROP(n, en_layer),                                                     \
        .layer_ru = DT_INST_PROP(n, ru_layer),                                                     \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_lang_init, NULL, &behavior_lang_data_##n,                  \
                            &behavior_lang_config_##n, APPLICATION,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_lang_driver_api);

DT_INST_FOREACH_STATUS_OKAY(LANG_INST)
