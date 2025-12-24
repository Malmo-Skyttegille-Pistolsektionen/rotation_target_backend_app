#include "display.h"
#include "input.h"
#include "timer.h"
#include "scene.h"

#include "app_version.h"
#include "utils.h"

#include "error_code.h"

#include <stdio.h>
#include <string.h>

typedef enum {
  MENU_STATE_STARTING,
  MENU_STATE_IDLE,
  MENU_STATE_SCENE_RUNNING,
} menu_state_t;

static struct {
  char          string[DISPLAY_SIZE_Y][DISPLAY_SIZE_X + 1];
  menu_state_t  state;

  int           scene;
} menu_data;

static void set_string(int row, const char* string)
{
  char*         s[ARRAY_SIZE(menu_data.string)];
  unsigned int  i;

  if (row < (int)ARRAY_SIZE(menu_data.string))
  {
    snprintf(menu_data.string[row], sizeof(menu_data.string[row]), "%s", string);
  }

  for (i = 0; i < ARRAY_SIZE(s); i++)
  {
    s[i] = menu_data.string[i];
  }
  display_set(s);
}

static void handle_input(input_event_t event)
{
  switch (event)
  {
    case INPUT_EVENT_UP:
      set_string(1, "up");
      break;

    case INPUT_EVENT_DOWN:
      set_string(1, "down");
      break;

    case INPUT_EVENT_LEFT:
      set_string(1, "left");
      break;

    case INPUT_EVENT_RIGHT:
      set_string(1, "right");
      break;
  }
}

static void handle_timer(void)
{
  switch (menu_data.state)
  {
    case MENU_STATE_STARTING:
      menu_data.state++;
      break;

    case MENU_STATE_IDLE:
      set_string(0, "Menu");
      set_string(1, scene_get_scene_name(menu_data.scene));
      break;

    case MENU_STATE_SCENE_RUNNING:
      break;
  }
}

error_code_t menu_init(void)
{
  char    version[10];

  memset(menu_data.string, 0, sizeof(menu_data.string));
  snprintf(version, sizeof(version), "%d.%d", APP_VERSION_MAJOR, APP_VERSION_MINOR);

  set_string(0, "revolvenow");
  set_string(1, version);

  input_register_callback(handle_input);
  timer_register_callback(handle_timer);

  return ERROR_CODE_OK;
}
