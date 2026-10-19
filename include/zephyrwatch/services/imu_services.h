/**
 * @file imu_services.h
 * @brief Interrupt-driven accelerometer sampling service.
 */

#ifndef IMU_SERVICES_H
#define IMU_SERVICES_H

#include <stdint.h>

#include <zephyr/kernel.h>

/**
 * @defgroup imu_service IMU service
 * @brief Acquires accelerometer samples and makes them available through a
 *        message queue.
 * @{
 */

/** @brief One three-axis accelerometer sample. */
typedef struct {
    /** Zephyr uptime, in milliseconds, when the sample was acquired. */
    int64_t timestamp_ms;

    /** X-axis acceleration in milli-g. */
    int32_t x_mg;

    /** Y-axis acceleration in milli-g. */
    int32_t y_mg;

    /** Z-axis acceleration in milli-g. */
    int32_t z_mg;

    /**
     * Monotonically increasing sample number.
     *
     * The value starts at one after initialization and wraps naturally after
     * UINT32_MAX.
     */
    uint32_t sequence;
} imu_accel_sample_t;

/**
 * @brief Initialize interrupt-driven accelerometer acquisition.
 *
 * The function clears queued samples and the dropped-sample counter, then
 * registers the accelerometer data-ready trigger.
 *
 * @retval 0 The service was initialized successfully.
 * @retval -ENODEV The IMU device is not ready.
 * @return A negative error returned by the sensor driver.
 */
int imu_service_init(void);

/**
 * @brief Read the oldest queued accelerometer sample.
 *
 * When the queue is full, the service discards its oldest sample so that the
 * newest measurement can be retained.
 *
 * @param[out] sample Destination for the accelerometer sample.
 * @param[in] timeout Maximum time to wait for a sample. Use @c K_NO_WAIT for
 *                    a non-blocking read or @c K_FOREVER to wait indefinitely.
 *
 * @retval 0 A sample was returned successfully.
 * @retval -EINVAL @p sample is NULL.
 * @return A negative error returned by the message queue.
 */
int imu_service_accel_read(imu_accel_sample_t* sample, k_timeout_t timeout);

/**
 * @brief Get the number of samples discarded because the queue was full.
 *
 * @return The dropped-sample count since the most recent initialization.
 */
uint32_t imu_service_dropped_samples_get(void);

/** @} */

#endif /* IMU_SERVICES_H */
