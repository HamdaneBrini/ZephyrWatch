/**
 * @file pedometer_app.h
 * @brief Detailed pedometer application interface.
 */

#ifndef PEDOMETER_APP_H
#define PEDOMETER_APP_H

#include <lvgl.h>

#include <zephyrwatch/core/app.h>

/**
 * @defgroup pedometer_app Pedometer application
 * @brief Displays the current step count and processes pending IMU samples.
 * @{
 */

/** @brief Registry entry for the pedometer application. */
extern const App_entry_t pedometer_app_entry;

/**
 * @brief Create the pedometer LVGL screen.
 *
 * @return Pointer to the newly created screen, or NULL if allocation fails.
 */
lv_obj_t* pedometer_app_create(void);

/**
 * @brief Start periodic pedometer processing and UI updates.
 *
 * @retval 0 The update timer was started successfully.
 * @retval -ENOMEM The LVGL timer could not be created.
 */
int pedometer_app_on_open(void);

/**
 * @brief Stop periodic pedometer processing and UI updates.
 *
 * @retval 0 The update timer was stopped successfully.
 * @retval -EINVAL No update timer is running.
 */
int pedometer_app_on_close(void);

/** @} */

#endif /* PEDOMETER_APP_H */
