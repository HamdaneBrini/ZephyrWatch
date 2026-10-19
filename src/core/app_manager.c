/**
 * @file app_manager.c
 * @brief Application lifecycle manager implementation.
 */

#include <stddef.h>
#include <sys/errno.h>

#include <zephyrwatch/applications/watchface/watchface_app.h>
#include <zephyrwatch/core/app_manager.h>
#include <zephyrwatch/core/app_registry.h>

/** Application currently loaded by the manager. */
static const App_entry_t* current_app;

/** LVGL screen belonging to the current application. */
static lv_obj_t* current_screen;

/**
 * @brief Invoke an application's open callback.
 *
 * @param app_entry Application to start.
 *
 * @return The callback result, or -EINVAL if no callback is available.
 */
static int start_app(const App_entry_t* app_entry) {
    if (app_entry != NULL && app_entry->on_open != NULL) {
        return app_entry->on_open();
    }

    return -EINVAL;
}

/** @copydoc app_manager_init */
int app_manager_init(void) {
    current_app = NULL;
    current_screen = NULL;

    return app_registry_register(&watchface_app_entry);
}

/** @copydoc app_manager_open */
int app_manager_open(const App_id_t app_id) {
    int ret;
    const App_entry_t* app;
    lv_obj_t* new_screen;

    ret = app_registry_get(app_id, &app);
    if (ret < 0) {
        return ret;
    }

    if (app->create == NULL) {
        return -ENOSYS;
    }

    lv_obj_clean(lv_scr_act());

    new_screen = app->create();

    if (new_screen == NULL) {
        return -ENOMEM;
    }

    lv_scr_load(new_screen);
    current_app = app;
    current_screen = new_screen;

    return start_app(app);
}
