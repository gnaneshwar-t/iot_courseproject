#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>

/**
 * @brief Bring up Wi-Fi in station mode and block until connected
 *        (or until a ~20 second timeout elapses).
 *        Uses a simple polling flag - no event groups, queues, or
 *        extra FreeRTOS tasks are created by this module.
 */
void wifi_manager_init_sta(const char *ssid, const char *password);

bool wifi_manager_is_connected(void);

#endif // WIFI_MANAGER_H
