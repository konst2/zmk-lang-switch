/*
 * Copyright (c) 2022 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

// текущий язык системы -- 0 английский, 1 -- русский
uint8_t get_language_state();
void set_language_state(uint8_t lang);
