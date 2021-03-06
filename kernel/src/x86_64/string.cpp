#include <string.hpp>

// Static, internal storage for printk().
static char internalbuf[32];

static void swap(char *a, char *b) {
    char temp = *a;
    *a = *b;
    *b = temp;
}

static void reverse(char *str, size_t length) {
    size_t end = length - 1;
    size_t start = 0;

    while(start < end) {
        swap(&str[start++], &str[end--]);
    }
}

static void pkargfinder(va_list ap, uintmax_t *stackdata, uint8_t bits) {
    switch(bits) {
        case 8: *stackdata = (uint8_t) va_arg(ap, uint32_t); break;
        case 16: *stackdata = (uint16_t) va_arg(ap, uint32_t); break;
        case 32: *stackdata = (uint32_t) va_arg(ap, uint32_t); break;
        case 64: *stackdata = (uint64_t) va_arg(ap, uint64_t); break;
        default: *stackdata = 0; break;
    }
}

size_t strlen(const char *str) {
    const char *start = str;
    while(*str) str++;
    return (str - start);
}

int strcmp(const char *x, const char *y) {
    while(*x && (*x == *y)) {
        x++; y++;
    }

    return *(const unsigned char*) x - *(const unsigned char*) y;
}

char *strcpy(char *dest, char *source) {
    char *temp = dest;
    while((*dest++ = *source++) != '\0');
    return temp;
}

char *strncpy(char *dest, char *source, size_t n) {
    char *temp = dest;
    while((*dest++ = *source++) != '\0' && --n > 0);
    *dest = '\0';
    return temp;
}

char *itoa(uintmax_t num, char *buffer, int base, bool pointer) {
    size_t index = 0;

    // As a quick hack, use goto when we're returning a 0x0 pointer.
    if(num == 0 && base == 16 && pointer) goto ptrpad;

    // Check for invalid parameters.
    else if(num == 0 || base < 2 || base > 32) {
        buffer[index++] = '0';
        buffer[index] = '\0';
        return buffer;
    }

    // Converts the number into ASCII in reverse order.
    while(num) {
        int rem = num % base;
        buffer[index++] = (rem >= 10) ? 65 + (rem - 10) : 48 + rem;
        num = num / base;
    }

    if(pointer) {
        ptrpad: // Is it a pointer?
        if(index < 16) while(index < 16) buffer[index++] = '0';
        buffer[index++] = 'x';
        buffer[index++] = '0';
    }

    // Terminate, reverse and return.
    buffer[index] = '\0';
    reverse(buffer, index);
    return buffer;
}

void printk(printk_output_t func, const char *format, va_list ap) {
    // Stack storage for the format string's variables.
    uintmax_t stackdata;
    uint8_t bits = 32;
    char *output;

    for(const char *fs = format; *fs; fs++) {
        // If *fs isn't the start of a parameter.
        if(*fs != '%') {
            func(*fs);
            continue;
        }

        swloop: switch(*++fs) {
            case 'l':  // Any length modifiers are dealt with here. Possible
            case 'z':  // data sizes as of now are 8, 16, 32 and 64 bits.
            case 'j':  // Case 'j' deals with intmax_t, which is a long.
            case 't':  // Case 't' deals with ptrdiff_t, which is also a long.
                bits = 64;
                goto swloop;
            case 'h':
                if(bits == 16) bits = 8;
                else bits = 16;
                goto swloop;

            case 'u':  // Here is where the data is retrieved from the list and
            case 'i':  // interpreted in itoa(), with appropriate arguments.
            case 'd':  // Specifiers with set bit length are at the bottom.
                pkargfinder(ap, &stackdata, bits);
                output = itoa(stackdata, internalbuf, 10, false);
                for(char *s = output; *s; s++) func(*s);
                break;
            case 'o':
                pkargfinder(ap, &stackdata, bits);
                output = itoa(stackdata, internalbuf, 8, false);
                for(char *s = output; *s; s++) func(*s);
                break;
            case 'x':
                pkargfinder(ap, &stackdata, bits);
                output = itoa(stackdata, internalbuf, 16, false);
                for(char *s = output; *s; s++) func(*s);
                break;
            case 'c':
                pkargfinder(ap, &stackdata, 8);
                func((char) stackdata);
                break;
            case 'p':
                pkargfinder(ap, &stackdata, 64);
                output = itoa(stackdata, internalbuf, 16, true);
                for(char *s = output; *s; s++) func(*s);
                break;
            case 's':
                pkargfinder(ap, &stackdata, 64);
                for(char *s = (char*) stackdata; *s; s++) func(*s);
                break;
            default:
                func(*fs);
                break;
        }
    }
}
