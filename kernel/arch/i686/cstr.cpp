#include <cstr.hpp>

namespace cstr {
    // Static storage for itoa().
    static char numascii[20];

    char *itoa(int num, int base) {
        bool negative = false;
        size_t index = 0;

        if(num == 0) {
            numascii[index++] = '0';
            numascii[index] = '\0';
            return numascii;
        } else if(num < 0 && base == 10) {
            negative = true;
            num = -num;
        }

        // Converts the number into ASCII in reverse order.
        while(num) {
            int rem = num % base;
            numascii[index++] = (rem > 9) ? (rem > 10) + 'a' : rem + '0';
            num = num / base;
        }

        if(negative) numascii[index++] = '-';
        numascii[index] = '\0';

        // Reverse and return.
        reverse(numascii, index);
        return numascii;
    }

    void reverse(char *str, size_t length) {
        size_t end = length - 1;
        size_t start = 0;

        while(start < end) {
            swap(&str[start++], &str[end--]);
        }
    }

    void swap(char *a, char *b) {
        char temp = *a;
        *a = *b;
        *b = temp;
    }

    size_t len(const char *str) {
        size_t len = 0;
        while(str[len]) len++;
        return len;
    }
}