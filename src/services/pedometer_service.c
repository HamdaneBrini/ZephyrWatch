/**
 * @file pedometer_service.c
 * @brief Accelerometer-based step detection service implementation.
 */

#include <stdbool.h>
#include <stdint.h>
#include <sys/errno.h>

#include <zephyr/kernel.h>

#include <zephyrwatch/services/pedometer_service.h>

/** Acceleration magnitude that arms the detector, squared milli-g. */
#define STEP_LOW_THRESHOLD_SQ  (INT64_C(1050) * INT64_C(1050))

/** Acceleration magnitude that triggers a step, squared milli-g. */
#define STEP_HIGH_THRESHOLD_SQ (INT64_C(1150) * INT64_C(1150))

/** Low-pass filter divisor. A larger value produces stronger smoothing. */
#define ACCEL_FILTER_DIVISOR   INT64_C(10)

/** Minimum interval between steps, equivalent to a maximum of 240 steps/min. */
#define MIN_STEP_INTERVAL_MS   INT64_C(1000)

/** Internal state of the step detector. */
typedef struct {
    int64_t filtered_magnitude_sq;
    int64_t last_sample_timestamp_ms;
    int64_t last_step_timestamp_ms;
    uint32_t step_count;
    bool initialized;
    bool armed;
} pedometer_state_t;

/** Current detector state, protected by pedometer_mutex. */
static pedometer_state_t pedometer_state;

/** Serializes access to the detector state. */
K_MUTEX_DEFINE(pedometer_mutex);

/**
 * @brief Calculate the squared acceleration-vector magnitude.
 *
 * Squared magnitude avoids a square-root operation while remaining independent
 * of watch orientation.
 *
 * @param sample Accelerometer sample to measure.
 *
 * @return Squared magnitude in milli-g squared.
 */
static int64_t acceleration_magnitude_sq(const imu_accel_sample_t* sample) {
    int64_t x = sample->x_mg;
    int64_t y = sample->y_mg;

    return x * x + y * y;
}

/** @copydoc pedometer_service_init */
int pedometer_service_init(void) {
    k_mutex_lock(&pedometer_mutex, K_FOREVER);

    pedometer_state.filtered_magnitude_sq = 0;
    pedometer_state.last_sample_timestamp_ms = 0;
    pedometer_state.last_step_timestamp_ms = -MIN_STEP_INTERVAL_MS;
    pedometer_state.step_count = 0;
    pedometer_state.initialized = false;
    pedometer_state.armed = false;

    k_mutex_unlock(&pedometer_mutex);
    return 0;
}

/** @copydoc pedometer_service_process_sample */
int pedometer_service_process_sample(const imu_accel_sample_t* sample) {
    int64_t magnitude_sq;
    int result = 0;

    if (sample == NULL || sample->timestamp_ms < 0) {
        return -EINVAL;
    }

    magnitude_sq = acceleration_magnitude_sq(sample);

    k_mutex_lock(&pedometer_mutex, K_FOREVER);

    if (!pedometer_state.initialized) {
        pedometer_state.filtered_magnitude_sq = magnitude_sq;
        pedometer_state.last_sample_timestamp_ms = sample->timestamp_ms;
        pedometer_state.initialized = true;
        pedometer_state.armed = magnitude_sq <= STEP_LOW_THRESHOLD_SQ;
        k_mutex_unlock(&pedometer_mutex);
        return 0;
    }

    if (sample->timestamp_ms < pedometer_state.last_sample_timestamp_ms) {
        k_mutex_unlock(&pedometer_mutex);
        return -EINVAL;
    }

    pedometer_state.last_sample_timestamp_ms = sample->timestamp_ms;
    pedometer_state.filtered_magnitude_sq +=
        (magnitude_sq - pedometer_state.filtered_magnitude_sq) / ACCEL_FILTER_DIVISOR;

    if (pedometer_state.filtered_magnitude_sq <= STEP_LOW_THRESHOLD_SQ) {
        pedometer_state.armed = true;
    } else if (pedometer_state.armed && pedometer_state.filtered_magnitude_sq >= STEP_HIGH_THRESHOLD_SQ
               && sample->timestamp_ms - pedometer_state.last_step_timestamp_ms >= MIN_STEP_INTERVAL_MS) {
        pedometer_state.step_count++;
        pedometer_state.last_step_timestamp_ms = sample->timestamp_ms;
        pedometer_state.armed = false;
        result = 1;
    }

    k_mutex_unlock(&pedometer_mutex);
    return result;
}

/** @copydoc pedometer_service_step_count_get */
uint32_t pedometer_service_step_count_get(void) {
    uint32_t step_count;

    k_mutex_lock(&pedometer_mutex, K_FOREVER);
    step_count = pedometer_state.step_count;
    k_mutex_unlock(&pedometer_mutex);

    return step_count;
}
