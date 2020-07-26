#include <gfx/renderer.hpp>
#include <mem/libc.hpp>
#include <logger.hpp>
#include <serial.hpp>
#include <string.hpp>
#include <timer.hpp>
#include <stdarg.h>
#include <stddef.h>

namespace log {
    metadata_t writers[LOGGER_MAX_OUTPUTS];
    size_t numwriters = 0;

    void registerwriter(writer_t function, bool trace, bool info, bool warn, bool error) {
        for(size_t i = 0; i < LOGGER_MAX_OUTPUTS; i++) {
            // If this slot isn't empty, go to the next one.
            if(writers[i].function != nullptr) continue;

            // Write the address and associated data.
            writers[i].function = function;
            writers[i].trace = trace;
            writers[i].info = info;
            writers[i].warn = warn;
            writers[i].error = error;
            numwriters++;
            break;
        }
    }

    void unregisterwriter(writer_t function) {
        for(size_t i = 0; i < LOGGER_MAX_OUTPUTS; i++) {
            // Set the function pointer to NULL if we find the one we want to clear.
            if(writers[i].function == function) {
                writers[i].function = nullptr;
                numwriters--;
            }
        }
    }

    void trace(const char *format, ...) {
        for(size_t i = 0; i < numwriters; i++) {
            if(writers[i].trace) {
                va_list ap; va_start(ap, format);
                printk(writers[i].function, format, ap);
                va_end(ap);
            }
        }
    }

    void info(const char *format, ...) {
        for(size_t i = 0; i < numwriters; i++) {
            if(writers[i].info) {
                va_list ap; va_start(ap, format);
                printk(writers[i].function, format, ap);
                va_end(ap);
            }
        }
    }

    void warn(const char *format, ...) {
        for(size_t i = 0; i < numwriters; i++) {
            if(writers[i].warn) {
                va_list ap; va_start(ap, format);
                printk(writers[i].function, format, ap);
                va_end(ap);
            }
        }
    }

    void error(const char *format, ...) {
        for(size_t i = 0; i < numwriters; i++) {
            if(writers[i].error) {
                va_list ap; va_start(ap, format);
                printk(writers[i].function, format, ap);
                va_end(ap);
            }
        }
    }

    void initialise(void) {
        // Zero out the logging vector table.
        memset(writers, 0, sizeof(metadata_t) * LOGGER_MAX_OUTPUTS);

        // Register the serial and graphical writers.
        registerwriter(&serial::writechar, true, true, true, true);
        registerwriter(&gfx::writechar, false, true, true, true);

        // Write some initialisation stuff to the log.
        trace("[logger] system log initialised at tick %d.\n", timer::systicks);
        trace("[logger] initial logger vector entries:\n", numwriters);
        for(size_t i = 0; i < numwriters; i++) trace("             -> %p\n", writers[i].function);
        trace("\n");
    }
}
