#include "httpd.h"
#include "httpd_file.h"
#include "network.h"

#include "utils.h"
#include "error_code.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define HTTPD_CONFIG_HANDLERS_MAX   40

typedef struct {
    httpd_method_t  method;
    char            service[HTTPD_SERVICE_PATH_LENGTH_MAX];
    httpd_handler_t callback;
    void*           userdata;
} handler_t;

static struct {
    network_client_t*   client;
    network_t*          network;
    handler_t           handlers[HTTPD_CONFIG_HANDLERS_MAX];
} httpd;

static error_code_t handler_create(handler_t** handler)
{
    unsigned int    i;

    for (i = 0; i < ARRAY_SIZE(httpd.handlers); i++)
    {
        if (httpd.handlers[i].method == HTTPD_METHOD_NONE)
        {
            *handler = &httpd.handlers[i];
            return ERROR_CODE_OK;
        }
    }

    return ERROR_CODE_HTTPD_NO_MORE_HANDLERS;
}

static error_code_t handler_free(handler_t* handler)
{
  if (handler < httpd.handlers || handler >= &httpd.handlers[ARRAY_SIZE(httpd.handlers)])
  {
    return ERROR_CODE_HTTPD_BAD_HANDLER;
  }

  memset(handler, 0, sizeof(*handler));
  handler->method = HTTPD_METHOD_NONE;

  return ERROR_CODE_OK;
}

static error_code_t handler_find(handler_t** handler, httpd_method_t method, const char* service)
{
    unsigned int    i;

    for (i = 0; i < ARRAY_SIZE(httpd.handlers); i++)
    {
        *handler = &httpd.handlers[i];

        if ((*handler)->method == method && strcmp((*handler)->service, service) == 0)
        {
             return ERROR_CODE_OK;
        }
    }

    *handler = NULL;

    return ERROR_CODE_HTTPD_NO_HANDLER;
}

static bool token_in_list(char token, char* list)
{
    char*   c;

    c = list;
    while (*c != '\0')
    {
        if (*c == token)
        {
            return true;
        }
        c++;
    }

    return false;
}

static error_code_t tokenize(char* buffer, uint32_t length, char* delimiters, int count, char** tokens, char** rest)
{
    char*   c;
    int     i;

    c = buffer;
    i = 0;
    while (i < count)
    {
        tokens[i] = c;
        while (!token_in_list(*c, delimiters) && c < &buffer[length])
        {
            c++;
        }

        if (c >= &buffer[length])
        {
            return ERROR_CODE_HTTPD_TOKINIZE_FAILED;
        }

        *c = '\0';
        c++;
        i++;
    }

    *rest = c;

    return ERROR_CODE_OK;
}

static void close_client(network_client_t* client, const char* message)
{
    (void)message;
#if DEBUG
    network_write(client, message, strlen(message));
    network_write(client, "\n", 1);
#endif // DEBUG

    network_close(client);
}

static httpd_method_t parse_method(const char* method)
{
    if (strcmp(method, "GET") == 0)
    {
        return HTTPD_METHOD_GET;
    }
    else if (strcmp(method, "POST") == 0)
    {
        return HTTPD_METHOD_POST;
    }
    else if (strcmp(method, "DELETE") == 0)
    {
        return HTTPD_METHOD_DELETE;
    }
    else
    {
        return HTTPD_METHOD_NONE;
    }
}

static void new_connection(network_client_t* client, void* userdata)
{
    uint8_t         buffer[512] = {0};
    uint32_t        length;
    char*           request[3];
    char*           data;
    error_code_t    res;
    handler_t*      handler;
    httpd_method_t  method;
    bool            done;

    (void)userdata;

    length = sizeof(buffer) - 1;
    res = network_read(client, buffer, &length);
    if (res != ERROR_CODE_OK)
    {
        close_client(client, "Read error");
        return;
    }

    // printf("\nRequest (%ld): %s\n", strlen((char*)buffer), buffer);

    res = tokenize((char*)buffer, length, " \n", 3, request, &data);
    if (res != ERROR_CODE_OK)
    {
        close_client(client, "Tokenize error");
        return;
    }

    printf("%s\n", request[1]);

    method = parse_method(request[0]);

    res = handler_find(&handler, method, request[1]);
    if (res != ERROR_CODE_OK)
    {
        res = httpd_file_send(client, request[1]);

        if (res == ERROR_CODE_OK)
        {
            close_client(client, "Done");
        }
        else
        {
            char    message[64];

            snprintf(message, sizeof(message), "%s not found\n", request[1]);

            httpd_send_header_error(client, 404);
            httpd_send_header_content(client, strlen(message), HTTPD_CONTENT_TYPE_TEXT);
            httpd_send_header_end(client);

            httpd_send_string(client, message);

            close_client(client, "Not found");
        }
    }
    else
    {
        done = handler->callback(client, request[1], data, handler->userdata);

        if (done)
        {
            close_client(client, "Done");
        }
    }
}

error_code_t httpd_register_handler(httpd_method_t method, const char* service, httpd_handler_t callback, void* userdata)
{
    handler_t*      handler;
    error_code_t    res;

    res = handler_create(&handler);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    // printf("Registering handler for: %s\n", service);

    handler->method = method;
    handler->callback = callback;
    handler->userdata = userdata;
    snprintf(handler->service, sizeof(handler->service), "%s", service);

    return ERROR_CODE_OK;
}

error_code_t httpd_unregister_handler(httpd_method_t method, const char* service)
{
    handler_t*      handler;
    error_code_t    res;

    res = handler_find(&handler, method, service);
    if (res != ERROR_CODE_OK)
    {
      return res;
    }

    res = handler_free(handler);
    if (res != ERROR_CODE_OK)
    {
      return res;
    }

    // printf("Unregistered handler for: %s\n", service);

    return ERROR_CODE_OK;
}

error_code_t httpd_start(void)
{

    error_code_t    res;

    res = network_create(&httpd.network);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    res = network_listen(httpd.network, 8080, new_connection, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    return ERROR_CODE_OK;
}

error_code_t httpd_init(void)
{
    return ERROR_CODE_OK;
}
