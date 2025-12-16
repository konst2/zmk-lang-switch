#include <zmk/language.h>

#include <stdint.h>

// текущий язык клавиатуры -- 0 английский, 1 -- русский
uint8_t kb_language_state = 0;
uint8_t get_kb_language() { 
    return kb_language_state; 
}
void set_kb_language(uint8_t lang) {
        kb_language_state = lang;
}


// текущий язык компьютера -- 0 английский, 1 -- русский
uint8_t os_language_state = 0;
uint8_t get_os_language() { 
    return os_language_state; 
}
void set_os_language(uint8_t lang) {
        os_language_state = lang;
}
