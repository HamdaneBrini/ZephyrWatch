/**
 * @file main.c
 * @brief ZephyrWatch firmware entry point.
 *
 * Initializes platform services and the watch face, then services LVGL.
 */

#include <stdint.h>

#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyrwatch/core/app_manager.h>
#include <zephyrwatch/platform/display.h>
#include <zephyrwatch/services/imu_services.h>
#include <zephyrwatch/services/pedometer_service.h>
#include <zephyrwatch/services/time_service.h>

/** CST8XX register controlling the controller's inactivity auto-sleep. */
#define CST8XX_REG_DISABLE_AUTO_SLEEP 0xFE

/** Non-zero value keeps the CST8XX awake while it is polled. */
#define CST8XX_AUTO_SLEEP_DISABLED    0x01

/** Touch controller selected by the board devicetree. */
static const struct device* touch_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_touch));

/** I2C bus and address of the board's touch controller. */
static const struct i2c_dt_spec touch_i2c = I2C_DT_SPEC_GET(DT_CHOSEN(zephyr_touch));

/** @brief Initialize ZephyrWatch and run its main event loop. */
int main(void) {
    if (!device_is_ready(touch_device)) {
        printk("Touch controller is not ready\n");
        return -1;
    }

    /* Polling cannot read the CST816S after it enters inactivity sleep. */
    if (i2c_reg_write_byte_dt(&touch_i2c, CST8XX_REG_DISABLE_AUTO_SLEEP, CST8XX_AUTO_SLEEP_DISABLED) != 0) {
        printk("Failed to disable touch controller auto-sleep\n");
        return -1;
    }

    if (time_service_init() != 0) {
        printk("Failed to initialize time service\n");
        return -1;
    }
    if (time_service_set(INT64_C(1760875200000), 2 * 60 * 60 * 1000, TIME_SOURCE_MANUAL) != 0) {
        printk("Failed to set test time\n");
        return -1;
    }
    if (imu_service_init() != 0) {
        printk("Failed to initialize IMU service\n");
        return -1;
    }
    if (pedometer_service_init() != 0) {
        printk("Failed to initialize pedometer service\n");
        return -1;
    }
    if (w_display_init() != 0) {
        printk("Failed to initialize display\n");
        return -1;
    }
    if (w_display_on() != 0) {
        printk("Failed to turn on display\n");
        return -1;
    }
    if (app_manager_init() != 0) {
        printk("Failed to initialize app manager\n");
        return -1;
    }
    if (app_manager_open(APP_ID_WATCHFACE) != 0) {
        printk("Failed to open watch face app\n");
        return -1;
    }

    while (1) {
        uint32_t delay_ms = lv_timer_handler();

        k_msleep(delay_ms);
    }

    return 0;
}
