#include <stdint.h>

extern "C" {
    uintptr_t __stack_chk_guard = 0xC001BEEFDEADBEEF;

    __attribute__((noreturn)) void __stack_chk_fail(void) {
        while(true) asm volatile("hlt");
    }
}
