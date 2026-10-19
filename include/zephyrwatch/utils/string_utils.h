/**
 * @file string_utils.h
 * @brief Integer-to-string conversion utilities.
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup string_utils String utilities
 * @brief Small string conversion helpers for the watch application.
 * @{
 */

/**
 * @brief Convert an unsigned 32-bit integer to decimal text.
 *
 * @param value Value to convert.
 * @param[out] result Destination buffer. It must contain at least 11 bytes to
 *                    hold any @c uint32_t value and the null terminator.
 *
 * @return Number of decimal digits written, excluding the null terminator.
 */
size_t uint_to_str(uint32_t value, char* result);

/** @} */

#endif /* STRING_UTILS_H */
