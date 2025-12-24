#include "scene.h"

#include "error_code.h"
#include "utils.h"

#include <stdio.h>

typedef enum {
  SCENE_TARGET_NONE,
  SCENE_TARGET_HIDE,
  SCENE_TARGET_SHOW,
} scene_target_t;

typedef enum {
  SCENE_AUDIO_NONE,
  SCENE_AUDIO_PLAY,
} scene_audio_t;

typedef struct {
  int             time;
  scene_target_t  target;
  scene_audio_t   audio;
  int             audio_file;
} scene_event_t;

typedef struct {
  char            name[DISPLAY_SIZE_X + 1];
  scene_event_t   events[20];
} scene_subscene_t;

typedef struct {
  char              name[DISPLAY_SIZE_X + 1];
  scene_subscene_t  subscene[3];
} scene_t;

static struct {
  struct {
    scene_t data[4];
    int     count;
  }       scenes;
  scene_t*  current;
} scene_data;

int scene_get_count(void)
{
  return scene_data.scenes.count;
}

char* scene_get_scene_name(int index)
{
  if (index >= scene_data.scenes.count)
  {
    return "";
  }

  return scene_data.scenes.data[index].name;
}

error_code_t scene_init(void)
{
  scene_data.scenes.count = ARRAY_SIZE(scene_data.scenes.data);

  snprintf(scene_data.scenes.data[0].name, sizeof(scene_data.scenes.data[0].name), "%s", "milsnabb");

  scene_data.scenes.data[0].subscene[0].events[0].time = 60;
  scene_data.scenes.data[0].subscene[0].events[1].time = 7;
  scene_data.scenes.data[0].subscene[0].events[2].time = 10;
  snprintf(scene_data.scenes.data[0].subscene[0].name,
           sizeof(scene_data.scenes.data[0].subscene[0].name),
           "%d s",
           scene_data.scenes.data[0].subscene[0].events[2].time);

  scene_data.scenes.data[0].subscene[1].events[0].time = 60;
  scene_data.scenes.data[0].subscene[1].events[1].time = 7;
  scene_data.scenes.data[0].subscene[1].events[2].time = 8;
  snprintf(scene_data.scenes.data[0].subscene[1].name,
           sizeof(scene_data.scenes.data[0].subscene[1].name),
           "%d s",
           scene_data.scenes.data[0].subscene[1].events[2].time);

  scene_data.scenes.data[0].subscene[2].events[0].time = 60;
  scene_data.scenes.data[0].subscene[2].events[1].time = 7;
  scene_data.scenes.data[0].subscene[2].events[2].time = 6;
  snprintf(scene_data.scenes.data[0].subscene[2].name,
           sizeof(scene_data.scenes.data[0].subscene[2].name),
           "%d s",
           scene_data.scenes.data[0].subscene[2].events[2].time);

  snprintf(scene_data.scenes.data[1].name, sizeof(scene_data.scenes.data[0].name), "%s", "snabbskjutning");

  scene_data.scenes.count++;

  scene_data.scenes.data[1].subscene[0].events[0].time = 10;
  scene_data.scenes.data[1].subscene[0].events[1].time = 3;
  scene_data.scenes.data[1].subscene[0].events[2].time = 7;
  scene_data.scenes.data[1].subscene[0].events[3].time = 3;
  scene_data.scenes.data[1].subscene[0].events[4].time = 7;
  scene_data.scenes.data[1].subscene[0].events[5].time = 3;
  scene_data.scenes.data[1].subscene[0].events[6].time = 7;
  scene_data.scenes.data[1].subscene[0].events[7].time = 3;
  scene_data.scenes.data[1].subscene[0].events[8].time = 7;
  scene_data.scenes.data[1].subscene[0].events[9].time = 3;
  scene_data.scenes.data[1].subscene[0].events[10].time = 7;
  scene_data.scenes.data[1].subscene[0].name[0] = '\0';

  scene_data.scenes.count++;

  return ERROR_CODE_OK;
}
