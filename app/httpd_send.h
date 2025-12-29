#ifndef HTTPD_SEND_H
#define HTTPD_SEND_H

#include "network.h"

#include <stdint.h>

typedef enum {
  HTTPD_CONTENT_TYPE_TEXT,
  HTTPD_CONTENT_TYPE_HTML,
  HTTPD_CONTENT_TYPE_CSS,
  HTTPD_CONTENT_TYPE_JS,
  HTTPD_CONTENT_TYPE_JSON,
  HTTPD_CONTENT_TYPE_PNG,
  HTTPD_CONTENT_TYPE_SVG,
  HTTPD_CONTENT_TYPE_SSE,
  HTTPD_CONTENT_TYPE_BIN,
} httpd_content_type_t;

void httpd_send_header_ok(network_client_t* client);
void httpd_send_header_error(network_client_t* client, int error_code);

void httpd_send_header_methods(network_client_t* client, const char* methods);
void httpd_send_header_content(network_client_t* client, uint32_t length, httpd_content_type_t type);
void httpd_send_header_end(network_client_t* client);

void httpd_send_string(network_client_t* client, const char* string);
void httpd_send_data(network_client_t* client, void* data, uint32_t length);

#endif // HTTPD_SEND_H
