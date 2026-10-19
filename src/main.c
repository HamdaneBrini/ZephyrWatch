/**
 * @file main.c
 * @brief ZephyrWatch firmware entry point.
 *
 * Initializes platform services and the watch face, then services LVGL.
 */

#include <stdint.h>

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyrwatch/core/app_manager.h>
#include <zephyrwatch/platform/display.h>
#include <zephyrwatch/services/imu_services.h>
#include <zephyrwatch/services/pedometer_service.h>
#include <zephyrwatch/services/time_service.h>

/** @brief Initialize ZephyrWatch and run its main event loop. */
int main(void) {
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
