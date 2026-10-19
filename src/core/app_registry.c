/**
 * @file app_registry.c
 * @brief Fixed-capacity application registry implementation.
 */

#include <stddef.h>
#include <sys/errno.h>

#include <zephyrwatch/core/app_registry.h>

/** Maximum number of applications held by the registry. */
#define MAX_APPS 10

/** Registered application entries. Unused slots contain NULL. */
static const App_entry_t* app_registry[MAX_APPS];

/** @copydoc app_registry_register */
int app_registry_register(const App_entry_t* entry) {
    if (entry == NULL) {
        return -EINVAL;
    }

    for (size_t i = 0; i < MAX_APPS; i++) {
        if (app_registry[i] == NULL) {
            app_registry[i] = entry;
            return 0;
        }
    }

    return -ENOMEM;
}

/** @copydoc app_registry_unregister */
int app_registry_unregister(const App_id_t id) {
    if (id >= APP_ID_COUNT) {
        return -EINVAL;
    }

    for (size_t i = 0; i < MAX_APPS; i++) {
        if (app_registry[i] != NULL && app_registry[i]->id == id) {
            app_registry[i] = NULL;
            return 0;
        }
    }

    return -ENOENT;
}

/** @copydoc app_registry_get */
int app_registry_get(const App_id_t id, const App_entry_t** entry) {
    if (id >= APP_ID_COUNT) {
        return -EINVAL;
    }
    if (entry == NULL) {
        return -EINVAL;
    }

    for (size_t i = 0; i < MAX_APPS; i++) {
        if (app_registry[i] != NULL && app_registry[i]->id == id) {
            *entry = app_registry[i];
            return 0;
        }
    }

    return -ENOENT;
}

/** @copydoc app_registry_count */
int app_registry_count(void) {
    int count = 0;

    for (size_t i = 0; i < MAX_APPS; i++) {
        if (app_registry[i] != NULL) {
            count++;
        }
    }

    return count;
}
