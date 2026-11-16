#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

/**
 * @brief Start the on-board HTTP server. Call only after Wi-Fi is
 *        connected (wifi_manager_is_connected() == true).
 */
void http_server_start(void);

#endif // HTTP_SERVER_H
