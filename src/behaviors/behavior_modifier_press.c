#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/behavior.h>
#include <zmk/endpoints.h>
#include <zmk/hid.h>
#include <zmk/language.h>
#include <zmk/events/keycode_state_changed.h>

#define DT_DRV_COMPAT zmk_behavior_modifier_press

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_modifier_press_config {};
struct behavior_modifier_press_data {};

static int behavior_modifier_press_init(const struct device *dev) {
    reset_modifiers_counter();
    return 0; 
}

static int modifier_press_binding_pressed(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    // 1. Увеличиваем счётчик нажатых модификаторов
    increment_modifiers_counter();

    // 2. Возвращаем нажатие самой клавиши сразу
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp);
}

static int modifier_press_binding_released(struct zmk_behavior_binding *binding,
                                            struct zmk_behavior_binding_event event) {
    // 1. Уменьшаем счётчик нажатых модификаторов
    decrement_modifiers_counter();

    // 2. Возвращаем отпускание самой клавиши
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp);
}

static const struct behavior_driver_api behavior_modifier_press_driver_api = {
    .binding_pressed = modifier_press_binding_pressed,
    .binding_released = modifier_press_binding_released,
};

#define MODIFIER_PRESS_INST(n)                                                                     \
    static struct behavior_modifier_press_data behavior_modifier_press_data_##n = {};                                  \
    static struct behavior_modifier_press_config behavior_modifier_press_config_##n = {};                              \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_modifier_press_init, NULL, &behavior_modifier_press_data_##n,                  \
                            &behavior_modifier_press_config_##n, APPLICATION,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_modifier_press_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MODIFIER_PRESS_INST)

