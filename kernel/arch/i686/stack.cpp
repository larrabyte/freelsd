#include <stdint.h>
#include <gfx.hpp>

extern "C" {
    uintptr_t __stack_chk_guard = 0xdeadbeef;  // find out how to randomise later

    // Only called when the stack smashing guard is overwritten.
    __attribute__ ((noreturn))
    void __stack_chk_fail(void) {
        #if __STDC_HOSTED__
            abort();
        #else
            vgatext::write("\n[ssp] Stack smashing guard overwritten.\n[ssp] Halting kernel execution.");
        #endif

        while(true) {  }
        __builtin_unreachable();
    }
}