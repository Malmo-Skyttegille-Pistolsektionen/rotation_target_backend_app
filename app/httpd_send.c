#include "httpd_send.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define HTTP_1_1              "HTTP/1.1 "
#define HTTP_200_OK           "200 OK\n"
#define HTTP_400_BAD_REQUEST  "400 Bad Request\n"
#define HTTP_404_NOT_FOUND    "404 Not Found\n"

#define CONNECTION_CLOSE   "Connection: close\n"
#define CONTENT_LENGTH     "Content-Length: "
#define CORS_ACL_ORIGIN    "Access-Control-Allow-Origin: *\n"

#define CONTENT_TYPE       "Content-Type: "
#define CONTENT_TYPE_TEXT  "text/plain"
#define CONTENT_TYPE_HTML  "text/html"
#define CONTENT_TYPE_CSS   "text/css"
#define CONTENT_TYPE_JS    "text/javascript"
#define CONTENT_TYPE_JSON  "application/json"
#define CONTENT_TYPE_PNG   "image/png"
#define CONTENT_TYPE_SVG   "image/svg+xml"
#define CONTENT_TYPE_SSE   "text/event-stream"
#define CONTENT_TYPE_BIN   "application/octet-stream"

void httpd_send_header_ok(network_client_t* client)
{
    httpd_send_string(client, HTTP_1_1);
    httpd_send_string(client, HTTP_200_OK);
    httpd_send_string(client, CONNECTION_CLOSE);
    httpd_send_string(client, CORS_ACL_ORIGIN);
}

void httpd_send_header_error(network_client_t* client, int error_code)
{
    httpd_send_string(client, HTTP_1_1);

    switch (error_code)
    {
      case 400:
        httpd_send_string(client, HTTP_400_BAD_REQUEST);
        break;

      case 404:
        httpd_send_string(client, HTTP_404_NOT_FOUND);
        break;
    }

    httpd_send_string(client, CONNECTION_CLOSE);
}

void httpd_send_header_content(network_client_t* client, uint32_t length, httpd_content_type_t type)
{
    char    string[11];

    if (length > 0)
    {
      snprintf(string, sizeof(string), "%u", length);

      httpd_send_string(client, CONTENT_LENGTH);
      httpd_send_string(client, string);
      httpd_send_string(client, "\n");
    }

    httpd_send_string(client, CONTENT_TYPE);
    switch (type)
    {
      case HTTPD_CONTENT_TYPE_TEXT:
        httpd_send_string(client, CONTENT_TYPE_TEXT);
        break;

      case HTTPD_CONTENT_TYPE_HTML:
        httpd_send_string(client, CONTENT_TYPE_HTML);
        break;

      case HTTPD_CONTENT_TYPE_CSS:
        httpd_send_string(client, CONTENT_TYPE_CSS);
        break;

      case HTTPD_CONTENT_TYPE_JS:
        httpd_send_string(client, CONTENT_TYPE_JS);
        break;

      case HTTPD_CONTENT_TYPE_JSON:
        httpd_send_string(client, CONTENT_TYPE_JSON);
        break;

      case HTTPD_CONTENT_TYPE_PNG:
        httpd_send_string(client, CONTENT_TYPE_PNG);
        break;

      case HTTPD_CONTENT_TYPE_SVG:
        httpd_send_string(client, CONTENT_TYPE_SVG);
        break;

      case HTTPD_CONTENT_TYPE_SSE:
        httpd_send_string(client, CONTENT_TYPE_SSE);
        break;

      case HTTPD_CONTENT_TYPE_BIN:
      default:
        httpd_send_string(client, CONTENT_TYPE_BIN);
        break;
    }
    httpd_send_string(client, "\n");
 }

void httpd_send_header_end(network_client_t* client)
{
    httpd_send_string(client, "\n");
}

void httpd_send_string(network_client_t* client, const char* string)
{
    network_write(client, string, strlen(string));
}

void httpd_send_data(network_client_t* client, void* data, uint32_t length)
{
    network_write(client, data, length);
}

