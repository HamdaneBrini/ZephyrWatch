/**
 * @file app_manager.h
 * @brief Application lifecycle manager API.
 */

#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <stdbool.h>

#include <zephyrwatch/core/app.h>

/**
 * @addtogroup app_core
 * @{
 */

/**
 * @brief Initialize the application manager and register built-in apps.
 *
 * @retval 0 The manager was initialized successfully.
 * @return A negative error returned by the application registry.
 */
int app_manager_init(void);

/**
 * @brief Create, load, and start an application.
 *
 * @param app_id Identifier of the application to open.
 *
 * @retval 0 The application was opened successfully.
 * @retval -ENOENT No application with @p app_id is registered.
 * @retval -ENOMEM The application screen could not be created.
 * @retval -ENOSYS The application has no screen creation callback.
 * @return A negative error returned by the application lifecycle callback.
 */
int app_manager_open(const App_id_t app_id);

/**
 * @brief Get the currently active application.
 *
 * @param[out] entry Destination for the active application entry.
 *
 * @retval 0 The active application was returned successfully.
 * @return A negative errno value if no application is active or @p entry is
 *         invalid.
 */
int app_manager_get_active_app(const App_entry_t** entry);

/**
 * @brief Test whether an application is currently active.
 *
 * @param app_id Identifier of the application to test.
 *
 * @retval true The application is active.
 * @retval false The application is not active.
 */
bool app_manager_is_app_active(const App_id_t app_id);

/** @} */

#endif /* APP_MANAGER_H */
