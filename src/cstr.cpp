#include "head/cstr.hpp"
#include <stddef.h>

size_t cstr::len(const char *str) {
    size_t len = 0;
    while(str[len]) len++;
    return len;
}