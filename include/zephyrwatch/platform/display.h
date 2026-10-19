/**
 * @file display.h
 * @brief ZephyrWatch display and backlight control API.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

/**
 * @defgroup zephyr_watch_display Display control
 * @brief Controls the display controller and its PWM backlight.
 * @{
 */

/**
 * @brief Initialize the display interface.
 *
 * Checks that both the display controller and the backlight PWM device are
 * ready. Call this function before using the other display API functions.
 *
 * @retval 0 The display interface is ready.
 * @retval -ENODEV The display controller or backlight PWM is not ready.
 */
int w_display_init(void);

/**
 * @brief Set the backlight brightness.
 *
 * @param percent Brightness percentage in the range 0 through 100.
 *
 * @retval 0 The brightness was updated.
 * @retval -EINVAL @p percent is greater than 100.
 * @return A negative errno value returned by the PWM driver on failure.
 */
int w_display_set_brightness(uint8_t percent);

/**
 * @brief Turn on the display and set the backlight to full brightness.
 *
 * A display driver that does not implement blanking is accepted; in that
 * case, only the backlight is enabled.
 *
 * @retval 0 The display is on.
 * @return A negative errno value returned by the display or PWM driver on
 *         failure.
 */
int w_display_on(void);

/**
 * @brief Turn off the backlight and blank the display.
 *
 * A display driver that does not implement blanking is accepted; the
 * backlight is still switched off.
 *
 * @retval 0 The display is off.
 * @return A negative errno value returned by the display or PWM driver on
 *         failure.
 */
int w_display_off(void);

/** @} */

#endif /* DISPLAY_H */
