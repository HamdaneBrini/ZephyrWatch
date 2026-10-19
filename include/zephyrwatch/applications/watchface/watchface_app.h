/**
 * @file watchface_app.h
 * @brief Main watch face application interface.
 */

#ifndef WATCHFACE_APP_H
#define WATCHFACE_APP_H

#include <lvgl.h>

#include <zephyrwatch/core/app.h>

/**
 * @defgroup watchface_app Watch face application
 * @brief Displays local time and the detected step count on the main screen.
 * @{
 */

/** @brief Registry entry for the watch face application. */
extern const App_entry_t watchface_app_entry;

/**
 * @brief Create the watch face LVGL screen.
 *
 * @return Pointer to the newly created screen, or NULL if allocation fails.
 */
lv_obj_t* watchface_create(void);

/**
 * @brief Start periodic clock updates and pedometer processing.
 *
 * @retval 0 The update timer was started successfully.
 * @retval -ENOMEM The LVGL timer could not be created.
 */
int watchface_on_open(void);

/**
 * @brief Stop periodic clock updates and pedometer processing.
 *
 * @retval 0 The update timer was stopped successfully.
 * @retval -EINVAL No update timer is running.
 */
int watchface_on_close(void);

/** @} */

#endif /* WATCHFACE_APP_H */
