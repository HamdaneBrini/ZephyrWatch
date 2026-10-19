
/**
 * @file time_service.c
 * @brief Monotonic-clock-based wall time service implementation.
 */

#include <sys/errno.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/timeutil.h>

#include <zephyrwatch/services/time_service.h>

/** Internal time synchronization state. */
typedef struct {
    /** UTC Unix timestamp recorded at synchronization, in milliseconds. */
    int64_t unix_ms_at_sync;

    /** Zephyr uptime recorded at synchronization, in milliseconds. */
    int64_t uptime_ms_at_sync;

    /** Signed local offset from UTC, in milliseconds. */
    int32_t utc_offset_ms;

    /** Source of the most recent synchronization. */
    time_source_t source;

    /** Whether the service currently contains valid synchronized time. */
    bool synchronized;
} time_state_t;

/** Current wall-time state. Access is protected by time_service_mutex. */
static time_state_t current_time;

/** Serializes access to current_time. */
K_MUTEX_DEFINE(time_service_mutex);

/** @copydoc time_service_init */
int time_service_init(void) {
    current_time.unix_ms_at_sync = 0;
    current_time.uptime_ms_at_sync = 0;
    current_time.utc_offset_ms = 0;
    current_time.source = TIME_SOURCE_NONE;
    current_time.synchronized = false;

    return 0;
}

/** @copydoc time_service_set */
int time_service_set(int64_t unix_time_ms, int32_t utc_offset_ms, time_source_t source) {
    if (source < TIME_SOURCE_NONE || source > TIME_SOURCE_RTC) {
        return -EINVAL;
    }
    if (utc_offset_ms < -86400000 || utc_offset_ms > 86400000) {
        return -EINVAL;
    }
    if (unix_time_ms < 0) {
        return -EINVAL;
    }
    k_mutex_lock(&time_service_mutex, K_FOREVER);
    current_time.unix_ms_at_sync = unix_time_ms;
    current_time.uptime_ms_at_sync = k_uptime_get();
    current_time.utc_offset_ms = utc_offset_ms;
    current_time.source = source;
    current_time.synchronized = true;
    k_mutex_unlock(&time_service_mutex);

    return 0;
}

/** @copydoc time_service_get_local */
int time_service_get_local(struct tm* local_time) {
    if (local_time == NULL) {
        return -EINVAL;
    }

    k_mutex_lock(&time_service_mutex, K_FOREVER);

    if (!current_time.synchronized) {
        k_mutex_unlock(&time_service_mutex);
        return -ENODATA;
    }

    int64_t elapsed_ms = k_uptime_get() - current_time.uptime_ms_at_sync;
    int64_t local_time_ms = current_time.unix_ms_at_sync + elapsed_ms + current_time.utc_offset_ms;
    time_t local_time_sec = (time_t)(local_time_ms / 1000);

    if (gmtime_r(&local_time_sec, local_time) == NULL) {
        k_mutex_unlock(&time_service_mutex);
        return -ERANGE;
    }

    k_mutex_unlock(&time_service_mutex);
    return 0;
}
