#include "http_status.h"

#include "httpd.h"
#include "program.h"

#include "error_code.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

#define HTTP_STATUS_CLIENTS_COUNT_MAX   10

static struct {
    network_client_t*   clients[HTTP_STATUS_CLIENTS_COUNT_MAX];
} http_status_data;


static void send_string(const char* string)
{
    unsigned int        i;
    network_client_t*   client;

    for (i = 0; i < ARRAY_SIZE(http_status_data.clients); i++)
    {
        client = http_status_data.clients[i];

        if (client != NULL)
        {
            httpd_send_string(client, string);
        }
    }
}

static void send_int(const char* tag, uint32_t value, bool last)
{
    char    string[16];

    snprintf(string, sizeof(string), "%d", value);

    send_string("\"");
    send_string(tag);
    send_string("\"");
    send_string(": ");
    send_string(string);

    if (!last)
    {
        send_string(", ");
    }
}

// status updates: https://github.com/jimisola/msg_rotation-target-web-app/blob/feature/initial_version/src/sse-client.js

static void program_status_listener(const program_status_data_t* status, void* userdata)
{
    (void)userdata;

    switch (status->type)
    {
        case PROGRAM_STATUS_TYPE_CHRONO:
            send_string("event: chrono");
            send_string("\n");

            send_string("data: ");
            send_string("{");
            send_int("elapsed", status->data.chrono.elapsed, false);
            send_int("remaining", status->data.chrono.remaining, false);
            send_int("total", status->data.chrono.total, true);
            send_string("}");
            send_string("\n");
            send_string("\n");
            break;

        case PROGRAM_STATUS_TYPE_PROGRAM_ADDED:
            break;

        case PROGRAM_STATUS_TYPE_PROGRAM_DELETED:
            break;

        case PROGRAM_STATUS_TYPE_PROGRAM_STARTED:
            send_string("event: program_started");
            send_string("\n");

            send_string("data: ");
            send_string("{");
            send_int("program_id", status->data.program_started.program_id, true);
            send_string("}");
            send_string("\n");
            send_string("\n");
            break;

        case PROGRAM_STATUS_TYPE_PROGRAM_COMPLETED:
            send_string("event: program_completed");
            send_string("\n");

            send_string("data: ");
            send_string("{");
            send_int("program_id", status->data.program_started.program_id, true);
            send_string("}");
            send_string("\n");
            send_string("\n");
            break;

        case PROGRAM_STATUS_TYPE_SERIES_STARTED:
            send_string("event: series_started");
            send_string("\n");

            send_string("data: ");
            send_string("{");
            send_int("program_id", status->data.series_started.program_id, false);
            send_int("series_index", status->data.series_started.series_index, true);
            send_string("}");
            send_string("\n");
            send_string("\n");
            break;

        case PROGRAM_STATUS_TYPE_SERIES_STOPPED:
            send_string("event: series_stopped");
            send_string("\n");

            send_string("data: ");
            send_string("{");
            send_int("program_id", status->data.series_stopped.program_id, false);
            send_int("series_index", status->data.series_stopped.series_index, false);
            send_int("event_index", status->data.series_stopped.event_index, true);
            send_string("}");
            send_string("\n");
            send_string("\n");
            break;

        case PROGRAM_STATUS_TYPE_SERIES_COMPLETED:
            send_string("event: series_completed");
            send_string("\n");

            send_string("data: ");
            send_string("{");
            send_int("program_id", status->data.series_completed.program_id, false);
            send_int("series_index", status->data.series_completed.series_index, true);
            send_string("}");
            send_string("\n");
            send_string("\n");
            break;

        case PROGRAM_STATUS_TYPE_SERIES_NEXT:
            send_string("event: series_next");
            send_string("\n");

            send_string("data: ");
            send_string("{");
            send_int("program_id", status->data.series_completed.program_id, false);
            send_int("series_index", status->data.series_completed.series_index, true);
            send_string("}");
            send_string("\n");
            send_string("\n");
            break;

        case PROGRAM_STATUS_TYPE_EVENT_STARTED:
            send_string("event: event_started");
            send_string("\n");

            send_string("data: ");
            send_string("{");
            send_int("program_id", status->data.event_started.program_id, false);
            send_int("series_index", status->data.event_started.series_index, false);
            send_int("event_index", status->data.event_started.event_index, true);
            send_string("}");
            send_string("\n");
            send_string("\n");
            break;

        case PROGRAM_STATUS_TYPE_TARGET_STATUS:
            break;

        case PROGRAM_STATUS_TYPE_AUDIO_ADDED:
            break;

        case PROGRAM_STATUS_TYPE_AUDIO_DELETED:
            break;
    }
}

static bool http_handler_sse(network_client_t* client, const char* service, const char* data, void* userdata)
{
    unsigned int        i;

    (void)data;
    (void)service;
    (void)userdata;

    for (i = 0; i < ARRAY_SIZE(http_status_data.clients); i++)
    {
        if (http_status_data.clients[i] == NULL)
        {
            http_status_data.clients[i] = client;

            httpd_send_header_ok(client);
            httpd_send_header_content(client, 0, HTTPD_CONTENT_TYPE_SSE);
            httpd_send_header_end(client);

            return false;
        }
    }

    httpd_send_header_error(client, 404);
    httpd_send_header_end(client);

    return true;
}

static bool http_handler_status(network_client_t* client, const char* service, const char* data, void* userdata)
{
  (void)data;
  (void)service;
  (void)userdata;

  httpd_send_header_ok(client);
  httpd_send_header_end(client);

  return true;
}

error_code_t http_status_init(void)
{
    error_code_t    res;

    res = httpd_register_handler(HTTPD_METHOD_GET, "/sse/v1", http_handler_sse, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    res = httpd_register_handler(HTTPD_METHOD_GET, "/api/v1/status", http_handler_status, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    res = program_register_status_listener(program_status_listener, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    return ERROR_CODE_OK;
}
