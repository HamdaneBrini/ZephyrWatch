
/**
 * @file string_utils.c
 * @brief Integer-to-string conversion utility implementation.
 */

#include <stddef.h>
#include <stdint.h>

#include <zephyrwatch/utils/string_utils.h>

/** @copydoc uint_to_str */
size_t uint_to_str(uint32_t value, char* result) {
    if (value == 0U) {
        result[0] = '0';
        result[1] = '\0';
        return 1;
    }

    uint8_t char_count = 0;
    uint32_t tmp = value;

    while (tmp != 0U) {
        char_count++;
        tmp /= 10U;
    }

    tmp = value;
    uint8_t i = 0;
    while (tmp != 0U) {
        result[char_count - 1U - i++] = (char)('0' + tmp % 10U);
        tmp /= 10U;
    }

    result[char_count] = '\0';
    return char_count;
}
