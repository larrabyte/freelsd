#include <string.hpp>

// Static storage for string functions.
static char internalbuf[256];

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

char *itoa(intmax_t num, int base) {
    bool negative = false;
    size_t index = 0;

    if(num < 0 && base == 10) {
        negative = true;
        num = -num;
    }

    // Check for invalid parameters.
    if(base < 2 || base > 32 || num == 0) {
        internalbuf[index++] = '0';
        internalbuf[index] = '\0';
        return internalbuf;
    }

    // Converts the number into ASCII in reverse order.
    while(num) {
        int rem = num % base;
        internalbuf[index++] = (rem >= 10) ? 65 + (rem - 10) : 48 + rem;
        num = num / base;
    }

    // Negative number?
    if(negative) internalbuf[index++] = '-';

    // Hex number?
    if(base == 16) {
        internalbuf[index++] = 'x';
        internalbuf[index++] = '0';
    }

    // Terminate, reverse and return.
    internalbuf[index] = '\0';
    reverse(internalbuf, index);
    return internalbuf;
}