/**
 * @file app_registry.h
 * @brief Application registration and lookup API.
 */

#ifndef APP_REGISTRY_H
#define APP_REGISTRY_H

#include <zephyrwatch/core/app.h>

/**
 * @addtogroup app_core
 * @{
 */

/**
 * @brief Add an application entry to the registry.
 *
 * @param entry Application entry to register. The registry stores the pointer,
 *              so the entry must remain valid while registered.
 *
 * @retval 0 The entry was registered successfully.
 * @retval -EINVAL @p entry is NULL.
 * @retval -ENOMEM The registry is full.
 */
int app_registry_register(const App_entry_t* entry);

/**
 * @brief Remove an application entry from the registry.
 *
 * @param app_id Identifier of the application to remove.
 *
 * @retval 0 The entry was removed successfully.
 * @retval -EINVAL @p app_id is outside the valid identifier range.
 * @retval -ENOENT The application is not registered.
 */
int app_registry_unregister(const App_id_t app_id);

/**
 * @brief Find a registered application by identifier.
 *
 * @param app_id Identifier of the application to find.
 * @param[out] entry Destination for the registered entry pointer.
 *
 * @retval 0 The application was found.
 * @retval -EINVAL An argument is invalid.
 * @retval -ENOENT The application is not registered.
 */
int app_registry_get(const App_id_t app_id, const App_entry_t** entry);

/**
 * @brief Get the number of registered applications.
 *
 * @return Number of entries currently stored in the registry.
 */
int app_registry_count(void);

/** @} */

#endif /* APP_REGISTRY_H */
