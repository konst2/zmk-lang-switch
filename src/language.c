#include <zmk/language.h>

#include <stdint.h>

// текущий язык системы -- 0 английский, 1 -- русский
uint8_t current_language_state = 0;
uint8_t get_language_state() { 
    return current_language_state; 
}
void set_language_state(uint8_t lang) {
        current_language_state = lang;
}
