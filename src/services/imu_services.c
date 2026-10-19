/**
 * @file imu_services.c
 * @brief Interrupt-driven QMI8658A accelerometer service implementation.
 */

#include <sys/errno.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

#include <zephyrwatch/services/imu_services.h>

/** QMI8658A devicetree node used by the service. */
#define QMI8658_NODE         DT_NODELABEL(qmi8658)

/** Maximum number of accelerometer samples buffered for consumers. */
#define ACCEL_QUEUE_CAPACITY 16

/** Queue transferring accelerometer samples from the trigger thread. */
K_MSGQ_DEFINE(accel_queue, sizeof(imu_accel_sample_t), ACCEL_QUEUE_CAPACITY, __alignof__(imu_accel_sample_t));

/** QMI8658A sensor device. */
static const struct device* const imu_device = DEVICE_DT_GET(QMI8658_NODE);

/** Number of samples discarded because the queue was full. */
static atomic_t dropped_samples;

/** Sequence number assigned to the next accelerometer sample. */
static uint32_t sample_sequence;

/** Data-ready trigger configuration for all three accelerometer axes. */
static const struct sensor_trigger accel_trigger = {
    .type = SENSOR_TRIG_DATA_READY,
    .chan = SENSOR_CHAN_ACCEL_XYZ,
};

/**
 * @brief Fetch and enqueue one accelerometer sample after a data-ready event.
 *
 * If the queue is full, the oldest queued sample is removed before the newest
 * one is inserted.
 *
 * @param device IMU device that generated the event.
 * @param trigger Trigger descriptor supplied by the sensor driver.
 */
static void accel_data_ready(const struct device* device, const struct sensor_trigger* trigger) {
    struct sensor_value acceleration[3];
    imu_accel_sample_t sample;

    ARG_UNUSED(trigger);

    if (sensor_sample_fetch_chan(device, SENSOR_CHAN_ACCEL_XYZ) < 0) {
        return;
    }

    if (sensor_channel_get(device, SENSOR_CHAN_ACCEL_XYZ, acceleration) < 0) {
        return;
    }

    sample.timestamp_ms = k_uptime_get();
    sample.x_mg = sensor_ms2_to_mg(&acceleration[0]);
    sample.y_mg = sensor_ms2_to_mg(&acceleration[1]);
    sample.z_mg = sensor_ms2_to_mg(&acceleration[2]);
    sample.sequence = ++sample_sequence;

    if (k_msgq_put(&accel_queue, &sample, K_NO_WAIT) < 0) {
        imu_accel_sample_t discarded;

        atomic_inc(&dropped_samples);
        if (k_msgq_get(&accel_queue, &discarded, K_NO_WAIT) == 0) {
            (void)k_msgq_put(&accel_queue, &sample, K_NO_WAIT);
        }
    }
}

/** @copydoc imu_service_init */
int imu_service_init(void) {
    if (!device_is_ready(imu_device)) {
        return -ENODEV;
    }

    k_msgq_purge(&accel_queue);
    atomic_clear(&dropped_samples);
    sample_sequence = 0;

    return sensor_trigger_set(imu_device, &accel_trigger, accel_data_ready);
}

/** @copydoc imu_service_accel_read */
int imu_service_accel_read(imu_accel_sample_t* sample, k_timeout_t timeout) {
    if (sample == NULL) {
        return -EINVAL;
    }

    return k_msgq_get(&accel_queue, sample, timeout);
}

/** @copydoc imu_service_dropped_samples_get */
uint32_t imu_service_dropped_samples_get(void) { return (uint32_t)atomic_get(&dropped_samples); }
