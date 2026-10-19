/**
 * @file time_service.h
 * @brief Monotonic-clock-based wall time service.
 */

#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/**
 * @defgroup time_service Time service
 * @brief Maintains local wall time from a synchronized Unix timestamp.
 * @{
 */

/** @brief Source used for the most recent time synchronization. */
typedef enum {
    TIME_SOURCE_NONE,   /**< Time has no external source. */
    TIME_SOURCE_BUILD,  /**< Time was initialized from the firmware build. */
    TIME_SOURCE_MANUAL, /**< Time was entered manually. */
    TIME_SOURCE_PHONE,  /**< Time was synchronized from a connected phone. */
    TIME_SOURCE_NTP,    /**< Time was synchronized through NTP. */
    TIME_SOURCE_RTC,    /**< Time was synchronized from a real-time clock. */
} time_source_t;

/**
 * @brief Reset the time service to its unsynchronized state.
 *
 * @retval 0 The service was initialized successfully.
 */
int time_service_init(void);

/**
 * @brief Synchronize the service with a Unix timestamp.
 *
 * The service advances the supplied timestamp using Zephyr uptime. Daylight
 * saving time is not calculated automatically and must already be reflected
 * in @p utc_offset_ms.
 *
 * @param unix_time_ms UTC milliseconds since the Unix epoch.
 * @param utc_offset_ms Signed local offset from UTC in milliseconds.
 * @param source Source of the time synchronization.
 *
 * @retval 0 The time was set successfully.
 * @retval -EINVAL A timestamp, offset, or source is invalid.
 */
int time_service_set(int64_t unix_time_ms, int32_t utc_offset_ms, time_source_t source);

/**
 * @brief Get the current local calendar time.
 *
 * @param[out] local_time Destination for the broken-down local time.
 *
 * @retval 0 The current local time was returned successfully.
 * @retval -EINVAL @p local_time is NULL.
 * @retval -ENODATA The service has not been synchronized.
 * @retval -ERANGE The current timestamp cannot be represented as calendar
 *                  time.
 */
int time_service_get_local(struct tm* local_time);

/** @} */

#endif /* TIME_SERVICE_H */
