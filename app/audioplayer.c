#include "audioplayer.h"

#include "config.h"
#include "audio.h"

#include "utils.h"
#include "error_code.h"

#include <string.h>

#define AUDIOPLAYER_PLAYLIST_COUNT_MAX CONFIG_AUDIOPLAYER_PLAYLIST_COUNT_MAX

static void play_current(void);

typedef enum {
  AUDIOPLAYER_STATE_IDLE,
  AUDIOPLAYER_STATE_PLAYING,
  AUDIOPLAYER_STATE_STOPPING,
} audioplayer_state_t;

typedef struct {
  uint8_t     count;
  uint8_t     current;
  audio_id_t  list[AUDIOPLAYER_PLAYLIST_COUNT_MAX];
} audioplayer_playlist_t;

static struct {
  audioplayer_state_t     state;
  audioplayer_playlist_t  playlist;
} audioplayer_data;

static void audio_done(void* userdata)
{
  (void)userdata;

  switch (audioplayer_data.state)
  {
    case AUDIOPLAYER_STATE_STOPPING:
      audioplayer_data.state = AUDIOPLAYER_STATE_PLAYING;
      break;

    case AUDIOPLAYER_STATE_PLAYING:
      audioplayer_data.playlist.current++;
      break;

    case AUDIOPLAYER_STATE_IDLE:
      return;
  }

  play_current();
}

static void play_current(void)
{
  if (audioplayer_data.playlist.current >= audioplayer_data.playlist.count)
  {
    audioplayer_data.state = AUDIOPLAYER_STATE_IDLE;
    return;
  }

  audio_play(audioplayer_data.playlist.list[audioplayer_data.playlist.current], audio_done, NULL);
}

error_code_t audioplayer_run(audio_id_t id[], uint8_t count)
{
  unsigned int  i;

  if (audioplayer_data.state != AUDIOPLAYER_STATE_IDLE)
  {
    audio_stop(audio_done, NULL);
    audioplayer_data.state = AUDIOPLAYER_STATE_STOPPING;
  }

  memset(&audioplayer_data.playlist, 0, sizeof(audioplayer_data.playlist));

  count = MIN(count, ARRAY_SIZE(audioplayer_data.playlist.list));
  audioplayer_data.playlist.count = count;
  for (i = 0; i < count; i++)
  {
    audioplayer_data.playlist.list[i] = id[i];
  }

  if (audioplayer_data.state == AUDIOPLAYER_STATE_IDLE)
  {
    audioplayer_data.state = AUDIOPLAYER_STATE_PLAYING;
    play_current();
  }

  return ERROR_CODE_OK;
}

error_code_t audioplayer_init(void)
{
  audioplayer_data.state = AUDIOPLAYER_STATE_IDLE;

  return ERROR_CODE_OK;
}
