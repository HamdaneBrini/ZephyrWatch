/**
 * @file app_manager.c
 * @brief Application lifecycle manager implementation.
 */

#include <stddef.h>
#include <sys/errno.h>

#include <zephyrwatch/applications/pedometer/pedometer_app.h>
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
    int ret;

    current_app = NULL;
    current_screen = NULL;

    ret = app_registry_register(&watchface_app_entry);
    if (ret < 0) {
        return ret;
    }

    ret = app_registry_register(&pedometer_app_entry);
    if (ret < 0) {
        (void)app_registry_unregister(APP_ID_WATCHFACE);
        return ret;
    }

    return 0;
}

/** @copydoc app_manager_open */
int app_manager_open(const App_id_t app_id) {
    int ret;
    const App_entry_t* app;
    const App_entry_t* previous_app = current_app;
    lv_obj_t* previous_screen = current_screen != NULL ? current_screen : lv_screen_active();
    lv_obj_t* new_screen;

    if (current_app != NULL && current_app->id == app_id) {
        return 0;
    }

    ret = app_registry_get(app_id, &app);
    if (ret < 0) {
        return ret;
    }

    if (app->create == NULL) {
        return -ENOSYS;
    }

    new_screen = app->create();
    if (new_screen == NULL) {
        return -ENOMEM;
    }

    if (previous_app != NULL && previous_app->on_close != NULL) {
        ret = previous_app->on_close();
        if (ret < 0) {
            lv_obj_delete(new_screen);
            return ret;
        }
    }

    lv_screen_load(new_screen);
    current_app = app;
    current_screen = new_screen;

    ret = start_app(app);
    if (ret < 0) {
        current_app = previous_app;
        current_screen = previous_app != NULL ? previous_screen : NULL;

        if (previous_screen != NULL) {
            lv_screen_load(previous_screen);
        }
        if (previous_app != NULL && previous_app->on_open != NULL) {
            (void)previous_app->on_open();
        }

        lv_obj_delete_async(new_screen);
        return ret;
    }

    if (previous_screen != NULL && previous_screen != new_screen) {
        lv_obj_delete_async(previous_screen);
    }

    return 0;
}

/** @copydoc app_manager_get_active_app */
int app_manager_get_active_app(const App_entry_t** entry) {
    if (entry == NULL) {
        return -EINVAL;
    }
    if (current_app == NULL) {
        return -ENOENT;
    }

    *entry = current_app;
    return 0;
}

/** @copydoc app_manager_is_app_active */
bool app_manager_is_app_active(const App_id_t app_id) { return current_app != NULL && current_app->id == app_id; }
