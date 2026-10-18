/**
 * @file display.c
 * @brief ZephyrWatch display and PWM backlight implementation.
 */

#include "display.h"

#include <stdint.h>
#include <sys/errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>

/** Display selected by the board's zephyr,display chosen node. */
static const struct device* display_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

/** PWM channel selected by the board's pwm-lcd0 alias. */
static const struct pwm_dt_spec backlight_pwm = PWM_DT_SPEC_GET(DT_ALIAS(pwm_lcd0));

/** @copydoc w_display_init */
int w_display_init(void) {
    if (!device_is_ready(display_device)) {
        printk("Display is not ready\n");
        return -ENODEV;
    }

    if (!pwm_is_ready_dt(&backlight_pwm)) {
        printk("Backlight PWM is not ready\n");
        return -ENODEV;
    }

    return 0;
}

/** @copydoc w_display_set_brightness */
int w_display_set_brightness(uint8_t percent) {
    uint32_t pulse;

    if (percent > 100U) {
        return -EINVAL;
    }

    pulse = (uint32_t)(((uint64_t)backlight_pwm.period * percent) / 100U);

    return pwm_set_pulse_dt(&backlight_pwm, pulse);
}

/** @copydoc w_display_on */
int w_display_on(void) {
    int ret = display_blanking_off(display_device);

    if (ret < 0 && ret != -ENOSYS) {
        return ret;
    }

    return w_display_set_brightness(100U);
}

/** @copydoc w_display_off */
int w_display_off(void) {
    int pwm_ret = w_display_set_brightness(0U);
    int display_ret = display_blanking_on(display_device);

    if (pwm_ret < 0) {
        return pwm_ret;
    }

    if (display_ret < 0 && display_ret != -ENOSYS) {
        return display_ret;
    }

    return 0;
}
