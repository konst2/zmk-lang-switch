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


LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/*
Behavior для переключения языков (EN/RU).
он позволяет переключаться между двумя языками и соответствующими слоями.
Использует глобальную переменную для хранения текущего состояния языка.
Параметры поведения:
- param1: Целевой язык (по номеру слоя -- обычно 0 - английский, 1 - русский, любое другое значение - переключить на противоположный)
Пример использования в keymap:
        ls: lang_switch {
            compatible = "zmk,behavior-lang-switch";
            #binding-cells = <1>;
            // Коды клавиш  F19-переключение на EN F18 = переключение на RU
            bindings = <&kp F19>, <&kp F18>;
            // раннее определенные слои для EN и RU
            en_layer = <ENGLISH>;
            ru_layer = <RUSSIAN>;
        };
*/
struct behavior_lang_config {
    struct zmk_behavior_binding behavior_ru;  // переключение на русский
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

    // 1. Определяем целевой язык
    // Проверяем параметр (layer_en - английский, layer_ru - русский, любое другое значение - переключить на противоположный)
    if (binding->param1 == config->layer_en) {
        target_language = config->layer_en; // Английский
    } 
    else if (binding->param1 == config->layer_ru) {
        target_language = config->layer_ru; // Русский
    }
    else {
        // Любое другое значение - переключить на противоположный язык
        if (get_kb_language() == config->layer_en) {
            target_language = config->layer_ru; // Переключить с английского на русский
        } else {
            target_language = config->layer_en; // Переключить с русского на английский
        }
    }
    
    // 2. Устанавливаем язык клавиатуры и переключаем слой
    if (get_kb_language() != target_language) {
        set_kb_language(target_language);
        zmk_keymap_layer_to(target_language, false);
    }


    // 3. Устанавливаем язык ОС
    // Если язык уже активен в ОС, ничего не делаем
    if (get_os_language() == target_language) {
        LOG_DBG("Language %d is already active", target_language);
        return ZMK_BEHAVIOR_OPAQUE;
    }


    // ### Нажимаем клавишу переключения в зависимости от выбранного языка
    if (target_language == config->layer_en) {
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
    set_os_language(target_language);
    // ###

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
