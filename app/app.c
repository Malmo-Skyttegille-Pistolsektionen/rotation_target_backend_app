#include "app.h"

#include "menu.h"
#include "httpd.h"
#include "httpd_file.h"
#include "http_api.h"
#include "program.h"

#include "utils.h"

#include "error_code.h"

typedef error_code_t (*init_function_t)(void);
typedef error_code_t (*start_function_t)(void);

static const init_function_t init_list[] = {
    menu_init,
    httpd_init,
    httpd_file_init,
    http_api_init,
    program_init,
};

static const start_function_t start_list[] = {
    httpd_start,
};

error_code_t app_start(void)
{
  error_code_t  res;
  unsigned int  i;

  for (i = 0; i < ARRAY_SIZE(start_list); i++)
  {
      res = start_list[i]();
      if (res != ERROR_CODE_OK)
      {
          return res;
      }
  }

  return ERROR_CODE_OK;
}

error_code_t app_init(void)
{
  error_code_t  res;
  unsigned int  i;

  for (i = 0; i < ARRAY_SIZE(init_list); i++)
  {
      res = init_list[i]();
      if (res != ERROR_CODE_OK)
      {
          return res;
      }
  }

  return ERROR_CODE_OK;
}

