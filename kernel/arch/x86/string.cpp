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

size_t strlen(const char *str) {
    const char *start = str;
    while(*str) str++;
    return (str - start);
}

char *itoa(intmax_t num, char *buffer, int base, bool ptrpad) {
    bool negative = false;
    size_t index = 0;

    // Only interpret negatives if it's base 10.
    if(num < 0 && base == 10) {
        negative = true;
        num = -num;
    }

    // As a quick hack, return 0x00000000 if we want a NULL pointer.
    else if(num == 0 && base == 16 && ptrpad) {
        return "00000000";
    }

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

    // Negative number?
    if(negative) buffer[index++] = '-';

    // Is it a pointer?
    if(ptrpad && index < 8) {
        while(index < 8) buffer[index++] = '0';
    }

    // Terminate, reverse and return.
    buffer[index] = '\0';
    reverse(buffer, index);
    return buffer;
}

void printk(printk_output_t func, const char *format, va_list ap) {
    // Stack storage for the format string's variables.
    unsigned int uints; int ints;
    char *strs; char chars;
    uintptr_t ptrs;

    for(const char *fs = format; *fs; fs++) {
        // If *fs isn't the start of a parameter.
        if(*fs != '%') {
            func(*fs);
            continue;
        }

        // Parameter specified?
        switch(*++fs) {
            case 'c':
                chars = (char) va_arg(ap, int);
                func(chars);
                break;
            case 'd':
            case 'i':
                ints = va_arg(ap, int);
                strs = itoa(ints, internalbuf, 10, false);
                for(char *s = strs; *s; s++) func(*s);
                break;
            case 'u':
                uints = va_arg(ap, unsigned int);
                strs = itoa(uints, internalbuf, 10, false);
                for(char *s = strs; *s; s++) func(*s);
                break;
            case 'o':
                uints = va_arg(ap, unsigned int);
                strs = itoa(uints, internalbuf, 8, false);
                for(char *s = strs; *s; s++) func(*s);
                break;
            case 'x':
                ptrs = va_arg(ap, unsigned int);
                strs = itoa(ptrs, internalbuf, 16, false);
                for(char *s = strs; *s; s++) func(*s);
                break;
            case 'p':
                ptrs = va_arg(ap, uintptr_t);
                strs = itoa(ptrs, internalbuf, 16, true);
                for(char *s = strs; *s; s++) func(*s);
                break;
            case 's':
                for(char *s = va_arg(ap, char*); *s; s++) func(*s);
                break;
            default:
                func(*fs);
                break;
        }
    }
}
