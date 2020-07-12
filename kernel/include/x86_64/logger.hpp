#ifndef FREELSD_KERNEL_LOGGER_HEADER
#define FREELSD_KERNEL_LOGGER_HEADER

#include <stddef.h>

#define LOGGER_MAX_OUTPUTS 256

namespace log {
    // Function prototype for a logging function.
    typedef void (*writer_t)(const char);

    typedef struct metadata {
        bool info, warn, error;
        writer_t function;
    } metadata_t;

    typedef enum infotype {
        LOGGER_INFO,
        LOGGER_WARN,
        LOGGER_ERROR
    } infotype_t;

    // The logger's writer function vector table.
    extern metadata_t writers[LOGGER_MAX_OUTPUTS];

    // The number of writers currently in the vector table.
    extern size_t numwriters;

    // Register a writer onto the logger's vector table.
    void registerwriter(writer_t function);

    // Write some information to the kernel log.
    void info(const char *format, ...);

    // Write a warning to the kernel log.
    void warn(const char *format, ...);

    // Write an error to the kernel log.
    void error(const char *format, ...);

    // Initialise the logging system.
    void initialise(void);
}

#endif
