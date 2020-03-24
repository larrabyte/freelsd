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

char *itoa(int num, int base) {
    bool negative = false;
    size_t index = 0;

    if(num == 0) {
        internalbuf[index++] = '0';
        internalbuf[index] = '\0';
        return internalbuf;
    } else if(num < 0 && base == 10) {
        negative = true;
        num = -num;
    }

    // Converts the number into ASCII in reverse order.
    while(num) {
        int rem = num % base;
        internalbuf[index++] = (rem > 9) ? (rem > 10) + 'a' : rem + '0';
        num = num / base;
    }

    if(negative) internalbuf[index++] = '-';
    internalbuf[index] = '\0';

    // Reverse and return.
    reverse(internalbuf, index);
    return internalbuf;
}