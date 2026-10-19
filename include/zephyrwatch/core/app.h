/**
 * @file app.h
 * @brief Common application identifiers and lifecycle interface.
 */

#ifndef APP_H
#define APP_H

#include <lvgl.h>
#include <sys/_intsup.h>

/**
 * @defgroup app_core Application core
 * @brief Defines applications and manages their lifecycle.
 * @{
 */

/** @brief Identifiers for applications available on the watch. */
typedef enum {
    APP_ID_WATCHFACE,   /**< Main watch face application. */
    APP_ID_SETTINGS,    /**< Watch settings application. */
    APP_ID_SYSTEM_INFO, /**< System information application. */

    APP_ID_COUNT /**< Number of application identifiers; must remain last. */
} App_id_t;

/** @brief Description and lifecycle callbacks for an application. */
typedef struct {
    /** Unique application identifier. */
    const App_id_t id;

    /** Human-readable application name. */
    const char* name;

    /** Callback invoked after the application screen is loaded. */
    int (*on_open)(void);

    /** Callback invoked before the application is closed. */
    int (*on_close)(void);

    /** Callback that creates and returns the application's LVGL screen. */
    lv_obj_t* (*create)(void);
} App_entry_t;

/** @} */

#endif /* APP_H */
