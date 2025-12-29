#include "http_api.h"
#include "http_status.h"

#include "httpd.h"
#include "program.h"
#include "programs.h"
#include "audio.h"

#include "error_code.h"
#include "utils.h"

#include "core_json.h"

#include <stdio.h>
#include <string.h>

static struct {
  uint32_t    series;
} api_data;

static bool http_handler_programs(network_client_t* client, const char* service, const char* data, void* userdata)
{
    error_code_t  res;
    const void*   buffer;
    uint32_t      length;
    uint32_t      count;
    unsigned int  i;
    program_id_t  id;

    static char   programs[1024];

    (void)service;
    (void)data;
    (void)userdata;

    memset(programs, 0, sizeof(programs));

    res = programs_get_count(&count);
    if (res == ERROR_CODE_OK)
    {
        snprintf(&programs[strlen(programs)], sizeof(programs) - strlen(programs), "[");

        for (i = 0; i < count; i++)
        {
            res = programs_get_by_index(i, &id, &buffer, &length);
            if (res == ERROR_CODE_OK)
            {
                JSONStatus_t  status;

                struct {
                  const char*   value;
                  size_t        length;
                  JSONTypes_t   type;
                } value;

                status = JSON_Validate(buffer, length);
                if (status != JSONSuccess)
                {
                    continue;
                }

                status = JSON_SearchConst(buffer, length, "title", strlen("title"), &value.value, &value.length, &value.type);
                if (status != JSONSuccess)
                {
                    continue;
                }

                snprintf(&programs[strlen(programs)], sizeof(programs) - strlen(programs),
                        "{"
                          "\"id\":%d,"
                          "\"title\":\"%.*s\""
                        "},",
                        id,
                        (int)value.length,
                        value.value);
            }
        }
        programs[strlen(programs) - 1] = '\0';
        snprintf(&programs[strlen(programs)], sizeof(programs) - strlen(programs), "]");
    }

    if (res == ERROR_CODE_OK)
    {
        httpd_send_header_ok(client);
        httpd_send_header_content(client, strlen(programs), HTTPD_CONTENT_TYPE_JSON);
        httpd_send_header_end(client);

        httpd_send_string(client, programs);
    }
    else
    {
        httpd_send_header_error(client, 404);
        httpd_send_header_end(client);
    }

    return true;
}

static bool http_handler_program_skip_to(network_client_t* client, const char* service, const char* data, void* userdata)
{
  uint32_t      id;
  error_code_t  res;

  (void)client;
  (void)service;
  (void)data;
  (void)userdata;

  id = (uint32_t)(uintptr_t)userdata;

  res = program_set_serie(id);
  if (res == ERROR_CODE_OK)
  {
    httpd_send_header_ok(client);
  }
  else
  {
    httpd_send_header_error(client, 404);
  }
  httpd_send_header_end(client);

  return true;
}

static bool http_handler_program_load(network_client_t* client, const char* service, const char* data, void* userdata)
{
    error_code_t  res;
    program_id_t  id;
    uint32_t      i;

    (void)service;
    (void)data;
    (void)userdata;

    id = (program_id_t)(uintptr_t)userdata;

    for (i = 0; i < api_data.series; i++)
    {
      char    path[HTTPD_SERVICE_PATH_LENGTH_MAX];

      snprintf(path, sizeof(path), "/api/v1/programs/series/%d/skip_to", i);
      res = httpd_unregister_handler(HTTPD_METHOD_POST, path);
    }

    res = program_load(id);
    if (res == ERROR_CODE_OK)
    {
      res = program_get_series_count(&api_data.series);
      if (res == ERROR_CODE_OK)
      {
        for (i = 0; i < api_data.series; i++)
        {
          char    path[HTTPD_SERVICE_PATH_LENGTH_MAX];

          snprintf(path, sizeof(path), "/api/v1/programs/series/%d/skip_to", i);
          res = httpd_register_handler(HTTPD_METHOD_POST, path, http_handler_program_skip_to, (void*)(uintptr_t)i);
        }
      }
      httpd_send_header_ok(client);
    }
    else
    {
      httpd_send_header_error(client, 404);
    }
    httpd_send_header_end(client);

    return true;
}

static bool http_handler_program_get(network_client_t* client, const char* service, const char* data, void* userdata)
{
    error_code_t    res;
    program_id_t    id;
    uint32_t        size;
    uint32_t        offset;
    uint32_t        sent;
    uint32_t        s;
    uint8_t         buffer[64];

    (void)service;
    (void)data;

    id = (program_id_t)(uintptr_t)userdata;

    res = programs_get_size(id, &size);
    if (res != ERROR_CODE_OK)
    {
        return true;
    }

    httpd_send_header_ok(client);
    httpd_send_header_content(client, size, HTTPD_CONTENT_TYPE_JSON);
    httpd_send_header_end(client);


    offset = 0;
    sent = 0;
    while (sent < size)
    {
        s = sizeof(buffer);
        res = programs_read(id, buffer, &s, offset);
        if (s > 0)
        {
            httpd_send_data(client, buffer, s);
            sent += s;
            offset = sent;
        }
    }

    return true;
}

static bool http_handler_program_start(network_client_t* client, const char* service, const char* data, void* userdata)
{
    error_code_t    res;

    (void)service;
    (void)data;
    (void)userdata;

    res = program_start();
    if (res == ERROR_CODE_OK)
    {
        httpd_send_header_ok(client);
    }
    else
    {
        httpd_send_header_error(client, 404);
    }
    httpd_send_header_end(client);

    return true;
}

static bool http_handler_program_stop(network_client_t* client, const char* service, const char* data, void* userdata)
{
    error_code_t    res;

    (void)service;
    (void)data;
    (void)userdata;

    res = program_stop();
    if (res == ERROR_CODE_OK)
    {
        httpd_send_header_ok(client);
    }
    else
    {
        httpd_send_header_error(client, 404);
    }
    httpd_send_header_end(client);

    return true;
}

static bool http_handler_program_show(network_client_t* client, const char* service, const char* data, void* userdata)
{
    (void)service;
    (void)data;
    (void)userdata;

    httpd_send_header_ok(client);
    httpd_send_header_end(client);

    return true;
}

static bool http_handler_program_hide(network_client_t* client, const char* service, const char* data, void* userdata)
{
    (void)service;
    (void)data;
    (void)userdata;

    httpd_send_header_ok(client);
    httpd_send_header_end(client);

    return true;
}

static bool http_handler_program_toggle(network_client_t* client, const char* service, const char* data, void* userdata)
{
    (void)service;
    (void)data;
    (void)userdata;

    httpd_send_header_ok(client);
    httpd_send_header_end(client);

    return true;
}

static bool http_handler_audios(network_client_t* client, const char* service, const char* data, void* userdata)
{
    error_code_t  res;
    uint32_t      count;
    unsigned int  i;
    audio_info_t  audio_info;

    static char   audios[1024] = {0};

    (void)service;
    (void)data;
    (void)userdata;

    res = audio_get_count(&count);
    snprintf(&audios[strlen(audios)], sizeof(audios) - strlen(audios), "{");

    snprintf(&audios[strlen(audios)], sizeof(audios) - strlen(audios), "\"builtin\":");
    snprintf(&audios[strlen(audios)], sizeof(audios) - strlen(audios), "[");
    if (res == ERROR_CODE_OK)
    {

        for (i = 0; i < count; i++)
        {
            res = audio_get_info(i, &audio_info);
            if (res == ERROR_CODE_OK)
            {
                snprintf(&audios[strlen(audios)], sizeof(audios) - strlen(audios),
                        "{"
                          "\"id\":%d,"
                          "\"title\":\"%s\""
                        "},",
                        audio_info.id,
                        audio_info.description);
            }
        }
        audios[strlen(audios) - 1] = '\0';
    }
    snprintf(&audios[strlen(audios)], sizeof(audios) - strlen(audios), "]");
    snprintf(&audios[strlen(audios)], sizeof(audios) - strlen(audios), ",");

    snprintf(&audios[strlen(audios)], sizeof(audios) - strlen(audios), "\"uploaded\":");
    snprintf(&audios[strlen(audios)], sizeof(audios) - strlen(audios), "[");
    snprintf(&audios[strlen(audios)], sizeof(audios) - strlen(audios), "]");

    snprintf(&audios[strlen(audios)], sizeof(audios) - strlen(audios), "}");

    if (res == ERROR_CODE_OK)
    {
        httpd_send_header_ok(client);
        httpd_send_header_content(client, strlen(audios), HTTPD_CONTENT_TYPE_JSON);
        httpd_send_header_end(client);

        httpd_send_string(client, audios);
    }
    else
    {
        httpd_send_header_error(client, 404);
        httpd_send_header_end(client);
    }

    return true;
}

error_code_t http_api_init(void)
{
    error_code_t    res;
    uint32_t        count;
    unsigned int    i;

    res = httpd_register_handler(HTTPD_METHOD_GET, "/api/v1/programs", http_handler_programs, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    res = httpd_register_handler(HTTPD_METHOD_POST, "/api/v1/programs/start", http_handler_program_start, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    res = httpd_register_handler(HTTPD_METHOD_POST, "/api/v1/programs/stop", http_handler_program_stop, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    res = httpd_register_handler(HTTPD_METHOD_POST, "/api/v1/programs/show", http_handler_program_show, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    res = httpd_register_handler(HTTPD_METHOD_POST, "/api/v1/programs/hide", http_handler_program_hide, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    res = httpd_register_handler(HTTPD_METHOD_POST, "/api/v1/programs/toggle", http_handler_program_toggle, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    programs_get_count(&count);
    for (i = 0; i < count; i++)
    {
        program_info_t  info;

        res = programs_get_info(i, &info);
        if (res == ERROR_CODE_OK)
        {
            char    path[HTTPD_SERVICE_PATH_LENGTH_MAX];

            snprintf(path, sizeof(path), "/api/v1/programs/%d", info.id);
            res = httpd_register_handler(HTTPD_METHOD_GET, path, http_handler_program_get, (void*)(uintptr_t)info.id);
            if (res != ERROR_CODE_OK)
            {
                return res;
            }

            snprintf(path, sizeof(path), "/api/v1/programs/%d/load", info.id);
            res = httpd_register_handler(HTTPD_METHOD_POST, path, http_handler_program_load, (void*)(uintptr_t)info.id);
            if (res != ERROR_CODE_OK)
            {
                return res;
            }
        }
    }

    res = httpd_register_handler(HTTPD_METHOD_GET, "/api/v1/audios", http_handler_audios, NULL);
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    res = http_status_init();
    if (res != ERROR_CODE_OK)
    {
        return res;
    }

    return ERROR_CODE_OK;
}
