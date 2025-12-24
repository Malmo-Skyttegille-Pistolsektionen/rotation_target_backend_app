#include "httpd_file.h"
#include "httpd.h"

#include "files.h"

#include "utils.h"
#include "error_code.h"

#include <stdio.h>
#include <string.h>

#define HTTPD_FILE_DEFAILT_INDEX    "index.html"

static const struct {
    const char*           file_extension;
    httpd_content_type_t  content_type;
} mime_types[] = {
    { "html",   HTTPD_CONTENT_TYPE_HTML  },
    { "htm",    HTTPD_CONTENT_TYPE_HTML  },
    { "css",    HTTPD_CONTENT_TYPE_CSS   },
    { "js",     HTTPD_CONTENT_TYPE_JS    },
    { "png",    HTTPD_CONTENT_TYPE_PNG   },
    { "svg",    HTTPD_CONTENT_TYPE_SVG   },
};

static httpd_content_type_t get_mime_type(const char* filename)
{
  char*         c;
  unsigned int  i;

  c = strrchr(filename, '.');

  if (c == NULL)
  {
    return HTTPD_CONTENT_TYPE_BIN;
  }

  c++;

  for (i = 0; i < ARRAY_SIZE(mime_types); i++)
  {
    if (strcmp(c, mime_types[i].file_extension) == 0)
    {
      return mime_types[i].content_type;
    }
  }

  return HTTPD_CONTENT_TYPE_BIN;
}

error_code_t httpd_file_send(network_client_t* client, const char* filename)
{
  error_code_t    res;
  uint32_t        length;
  uint8_t         buffer[32];

  while (*filename == '/')
  {
    filename++;
  }

  if (*filename == '\0')
  {
    filename = HTTPD_FILE_DEFAILT_INDEX;
  }

  // printf("Fetching %s\n", filename);

  res = files_open(filename);
  if (res != ERROR_CODE_OK)
  {
    return res;
  }

  res = files_get_size(&length);
  if (res != ERROR_CODE_OK)
  {
    return res;
  }

  httpd_send_header_ok(client);
  httpd_send_header_content(client, length, get_mime_type(filename));
  httpd_send_header_end(client);

  do {
    length = sizeof(buffer);
    res = files_read(buffer, &length);
    if (res == ERROR_CODE_OK)
    {
      httpd_send_data(client, buffer, length);
    }
  } while (length > 0);

  files_close();

  return ERROR_CODE_OK;
}

error_code_t httpd_file_init(void)
{
  return ERROR_CODE_OK;
}
