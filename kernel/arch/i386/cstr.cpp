#include <cstr.hpp>

char *cstr::itoa(int64_t num, char *str, uint64_t base) {
    bool negative = false;
    size_t index = 0;

    if(num == 0) {
        str[index++] = '0';
        str[index] = '\0';
        return str;
    } else if(num < 0 && base == 10) {
        negative = true;
        num = -num;
    }

    // Converts the number into ASCII in reverse order.
    while(num) {
        int rem = num % base;
        str[index++] = (rem > 9) ? (rem > 10) + 'a' : rem + '0';
        num = num / base;
    }

    if(negative) str[index++] = '-';
    str[index] = '\0';

    // Reverse and return.
    reverse(str, index);
    return str;
}

void cstr::reverse(char *str, uint64_t length) {
    uint64_t start = 0;
    uint64_t end = length - 1;

    while(start < end) {
        swap(&str[start++], &str[end--]);
    }
}

void cstr::swap(char *a, char *b) {
    char temp = *a;
    *a = *b;
    *b = temp;
}

size_t cstr::len(const char *str) {
    size_t len = 0;
    while(str[len]) len++;
    return len;
}