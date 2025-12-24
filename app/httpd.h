#ifndef HTTPD_H
#define HTTPD_H

#include "httpd_send.h"

#include "error_code.h"
#include "network.h"

#include <stdbool.h>

#define HTTPD_SERVICE_PATH_LENGTH_MAX   40

typedef bool (*httpd_handler_t)(network_client_t* client, const char* service, const char* data, void* userdata);

typedef enum {
    HTTPD_METHOD_NONE,
    HTTPD_METHOD_GET,
    HTTPD_METHOD_POST,
    HTTPD_METHOD_DELETE,
} httpd_method_t;

error_code_t httpd_register_handler(httpd_method_t method, const char* service, httpd_handler_t handler, void* userdata);
error_code_t httpd_unregister_handler(httpd_method_t method, const char* service);

error_code_t httpd_start(void);
error_code_t httpd_init(void);

#endif // HTTPD_H
