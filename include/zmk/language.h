/*
 * Copyright (c) 2022 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

// текущий язык клавиатуры -- 0 английский, 1 -- русский
uint8_t get_kb_language();
void set_kb_language(uint8_t lang);

// текущий язык компьютера -- 0 английский, 1 -- русский
uint8_t get_os_language();
void set_os_language(uint8_t lang);
