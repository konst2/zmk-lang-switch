#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/keymap.h>
#include <zmk/behavior.h>
#include <zmk/endpoints.h>
#include <zmk/hid.h>
#include <zmk/language.h>
#include <zmk/events/keycode_state_changed.h>

#define DT_DRV_COMPAT zmk_behavior_modifier_press

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_modifier_press_config {
    struct zmk_behavior_binding behavior_ru;  // переключение на русский
    struct zmk_behavior_binding behavior_en;  // переключение на английский
    uint8_t layer_en; // слой EN
    uint8_t layer_ru; // слой RU
};

struct behavior_modifier_press_data {};

static int behavior_modifier_press_init(const struct device *dev) {
    reset_modifiers_counter();
    return 0; 
}

static int modifier_press_binding_pressed(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_modifier_press_config *config = dev->config;

    // 1. Увеличиваем счётчик нажатых модификаторов
    increment_modifiers_counter();

    if (get_modifiers_counter() == 1) {
        // 2. запоминаем текущий язык клавиатуры
        set_kb_language_before_modifiers(get_kb_language());

        if (zmk_keymap_layer_active(config->layer_ru)) {
        // 3. переключаем язык клавиатуры и ОС на английский, меняем слой
            set_kb_language(config->layer_en);
            zmk_keymap_layer_to(config->layer_en, false);
            switch_os_language(config->layer_en, config->layer_en, config->behavior_ru, config->behavior_en, event);
        }
    }

    // 4. Возвращаем нажатие самой клавиши сразу
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp);
}

static int modifier_press_binding_released(struct zmk_behavior_binding *binding,
                                            struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_modifier_press_config *config = dev->config;

    // 1. Уменьшаем счётчик нажатых модификаторов
    decrement_modifiers_counter();

    if (get_modifiers_counter() == 0 && get_kb_language_before_modifiers() == config->layer_ru) {
        // 2. переключаем язык клавиатуры и ОС на русский, меняем слой
        set_kb_language(config->layer_ru);
        zmk_keymap_layer_to(config->layer_ru, false);
        switch_os_language(config->layer_ru, config->layer_en, config->behavior_ru, config->behavior_en, event);
    }

    // 2. Возвращаем отпускание самой клавиши
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp);
}

static const struct behavior_driver_api behavior_modifier_press_driver_api = {
    .binding_pressed = modifier_press_binding_pressed,
    .binding_released = modifier_press_binding_released,
};

#define MODIFIER_PRESS_INST(n)                                                                         \
    static struct behavior_modifier_press_data behavior_modifier_press_data_##n = {};                  \
    static struct behavior_modifier_press_config behavior_modifier_press_config_##n = {                \
        .behavior_en = ZMK_KEYMAP_EXTRACT_BINDING(0, DT_DRV_INST(n)),                                  \
        .behavior_ru = ZMK_KEYMAP_EXTRACT_BINDING(1, DT_DRV_INST(n)),                                  \
        .layer_en = DT_INST_PROP(n, en_layer),                                                         \
        .layer_ru = DT_INST_PROP(n, ru_layer),                                                         \
    };                                                                                                 \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_modifier_press_init, NULL, &behavior_modifier_press_data_##n,  \
                            &behavior_modifier_press_config_##n, APPLICATION,                          \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_modifier_press_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MODIFIER_PRESS_INST)

