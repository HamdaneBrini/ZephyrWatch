/**
 * @file pedometer_service.h
 * @brief Accelerometer-based step detection service.
 */

#ifndef PEDOMETER_SERVICE_H
#define PEDOMETER_SERVICE_H

#include <stdint.h>

#include <zephyrwatch/services/imu_services.h>

/**
 * @defgroup pedometer_service Pedometer service
 * @brief Detects steps from the X/Y plane of timestamped accelerometer samples.
 * @{
 */

/**
 * @brief Reset the detector and step count.
 *
 * Call this function once before processing accelerometer samples.
 *
 * @retval 0 The service was initialized successfully.
 */
int pedometer_service_init(void);

/**
 * @brief Process one accelerometer sample.
 *
 * The detector uses planar acceleration magnitude, low-pass filtering,
 * threshold hysteresis, and a minimum step interval. Samples must be supplied
 * in nondecreasing timestamp order.
 *
 * @param sample Accelerometer sample to process.
 *
 * @retval 1 A new step was detected.
 * @retval 0 No step was detected.
 * @retval -EINVAL @p sample is NULL, has a negative timestamp, or is older
 *                  than the previously processed sample.
 */
int pedometer_service_process_sample(const imu_accel_sample_t* sample);

/**
 * @brief Process all accelerometer samples currently queued by the IMU service.
 *
 * This non-blocking function is intended to be called periodically by the
 * active application. Each queued sample is passed to
 * pedometer_service_process_sample().
 *
 * @return Number of samples processed, or a negative errno value on failure.
 */
int pedometer_service_process_pending(void);

/**
 * @brief Get the detected step count.
 *
 * @return Number of detected steps since the most recent initialization.
 */
uint32_t pedometer_service_step_count_get(void);

/** @} */

#endif /* PEDOMETER_SERVICE_H */
