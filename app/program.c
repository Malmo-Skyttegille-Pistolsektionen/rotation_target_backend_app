#include "program.h"

#include "config.h"

#include "audioplayer.h"
#include "programs.h"
#include "timer.h"
#include "targets.h"

#include "error_code.h"
#include "utils.h"

#include "core_json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_SERIE_AUDIO_COUNT_MAX   CONFIG_AUDIOPLAYER_PLAYLIST_COUNT_MAX
#define PROGRAM_SERIE_EVENT_COUNT_MAX   CONFIG_PROGRAM_SERIE_EVENT_COUNT_MAX
#define PROGRAM_SERIE_COUNT_MAX         CONFIG_PROGRAM_SERIE_COUNT_MAX

#define PROGRAM_STATUS_LISTENER_COUNT_MAX   4
#define PROGRAM_STATUS_UPDATE_INTERVAL_MS   1000

typedef enum {
    COMMAND_NONE,
    COMMAND_SHOW,
    COMMAND_HIDE,
} command_t;

typedef enum {
    PROGRAM_STATE_IDLE,
    PROGRAM_STATE_STARTING,
    PROGRAM_STATE_RUNNING,
    PROGRAM_STATE_STOPPING,
} program_state_t;

typedef struct {
    program_status_callback_t   callback;
    void*                       userdata;
} status_callback_t;

typedef struct {
    uint8_t     count;
    audio_id_t  id[PROGRAM_SERIE_AUDIO_COUNT_MAX];
} audio_t;

typedef struct {
    uint32_t        start_time;
    uint16_t        duration;
    command_t       command;
    targets_bank_t  bank;
    audio_t         audio;
} event_t;

typedef struct {
    bool            optional;
    uint16_t        count;
    uint32_t        total_time;
    event_t         event[PROGRAM_SERIE_EVENT_COUNT_MAX];
} serie_t;

typedef struct {
    program_id_t    id;
    uint16_t        count;
    serie_t         serie[PROGRAM_SERIE_COUNT_MAX];
} program_t;

typedef struct {
    const char* value;
    size_t      length;
    JSONTypes_t type;
} json_value_t;

typedef struct {
    const void*   buffer;
    uint32_t      length;
    size_t        start;
    size_t        next;
    JSONPair_t    pair;
} json_iterator_t;

static struct {
    program_state_t state;
    struct {
        uint32_t        time;
        uint32_t        last_update;
        serie_t*        serie;
        event_t*        event;
    }               running;
    program_t       program;
    status_callback_t   status_callbacks[PROGRAM_STATUS_LISTENER_COUNT_MAX];
} program_data;

static uint16_t event_index(serie_t* serie, event_t* event)
{
    return ((uintptr_t)event - (uintptr_t)serie->event) / sizeof(*event);
}

static event_t* next_event(serie_t* serie, event_t* event)
{
    event++;

    if (event >= &serie->event[serie->count])
    {
        return NULL;
    }

    return event;
}

static uint16_t serie_index(program_t* program, serie_t* serie)
{
    return ((uintptr_t)serie - (uintptr_t)program->serie) / sizeof(*serie);
}

static serie_t* next_serie(program_t* program, serie_t* serie)
{
    serie++;

    if (serie >= &program->serie[program->count])
    {
        return NULL;
    }

    return serie;
}

static void call_status_callbacks(const program_status_data_t* status)
{
    unsigned int        i;
    status_callback_t*  current;

    for (i = 0; i < ARRAY_SIZE(program_data.status_callbacks); i++)
    {
        current = &program_data.status_callbacks[i];

        if (current->callback != NULL)
        {
            current->callback(status, current->userdata);
        }
    }
}

static void activate_current_event(void)
{
    program_t*  program;
    serie_t*    serie;
    event_t*    event;

    program_status_data_t   status;

    program = &program_data.program;
    serie = program_data.running.serie;
    event = program_data.running.event;

    status.type = PROGRAM_STATUS_TYPE_EVENT_STARTED;
    status.data.event_started.program_id = program->id;
    status.data.event_started.series_index = serie_index(program, serie);
    status.data.event_started.event_index = event_index(serie, event);

    if (event->audio.count > 0)
    {
      audioplayer_run(event->audio.id, event->audio.count);
    }

    switch (event->command)
    {
      case COMMAND_SHOW:
        targets_show(event->bank);
        break;

      case COMMAND_HIDE:
        targets_hide(event->bank);
        break;

       case COMMAND_NONE:
        break;
    }

    call_status_callbacks(&status);
}

static void send_status_chrono(uint32_t elapsed, serie_t* serie)
{
    program_status_data_t   status;

    status.type = PROGRAM_STATUS_TYPE_CHRONO;
    status.data.chrono.elapsed = elapsed;
    if (serie == NULL)
    {
        status.data.chrono.remaining = 0;
        status.data.chrono.total = 0;
    }
    else
    {
        status.data.chrono.remaining = serie->total_time - elapsed;
        status.data.chrono.total = serie->total_time;
    }

    call_status_callbacks(&status);
}

static void program_timer(void)
{
    event_t*    event;
    serie_t*    serie;

    program_status_data_t   status;

    serie = program_data.running.serie;
    event = program_data.running.event;

    switch (program_data.state)
    {
        case PROGRAM_STATE_IDLE:
            return;

        case PROGRAM_STATE_STARTING:
            program_data.running.time = 0;
            program_data.running.last_update = 0;
            program_data.running.event = &serie->event[0];
            event = program_data.running.event;

            program_data.state = PROGRAM_STATE_RUNNING;

            status.type = PROGRAM_STATUS_TYPE_PROGRAM_STARTED;
            status.data.program_started.program_id = program_data.program.id;
            call_status_callbacks(&status);

            status.type = PROGRAM_STATUS_TYPE_SERIES_STARTED;
            status.data.series_started.program_id = program_data.program.id;
            status.data.series_started.series_index = serie_index(&program_data.program, serie);
            call_status_callbacks(&status);

            send_status_chrono(program_data.running.time, serie);

            activate_current_event();
            break;

        case PROGRAM_STATE_RUNNING:
            program_data.running.time += CONFIG_TIMER_TICK_INTERVAL_MS;

            if (program_data.running.time - program_data.running.last_update >= PROGRAM_STATUS_UPDATE_INTERVAL_MS)
            {
                send_status_chrono(program_data.running.time, serie);
                program_data.running.last_update = program_data.running.time;
            }

            if (program_data.running.time >= event->start_time + event->duration)
            {
                program_data.running.event = next_event(serie, event);
                event = program_data.running.event;
                if (event == NULL)
                {
                    program_data.state = PROGRAM_STATE_IDLE;

                    status.type = PROGRAM_STATUS_TYPE_SERIES_COMPLETED;
                    status.data.series_completed.program_id = program_data.program.id;
                    status.data.series_completed.series_index = serie_index(&program_data.program, serie);
                    call_status_callbacks(&status);

                    send_status_chrono(0, NULL);

                    program_data.running.serie = next_serie(&program_data.program, serie);
                    serie = program_data.running.serie;
                    if (program_data.running.serie == NULL)
                    {
                        program_data.running.serie = &program_data.program.serie[0];
                        serie = program_data.running.serie;

                        status.type = PROGRAM_STATUS_TYPE_PROGRAM_COMPLETED;
                        status.data.program_completed.program_id = program_data.program.id;
                        call_status_callbacks(&status);
                    }

                    status.type = PROGRAM_STATUS_TYPE_SERIES_NEXT;
                    status.data.series_next.program_id = program_data.program.id;
                    status.data.series_next.series_index = serie_index(&program_data.program, serie);
                    call_status_callbacks(&status);

                    program_data.running.event = &program_data.running.serie->event[0];
                    event = program_data.running.event;
                    return;
                }

                activate_current_event();
            }
            break;

        case PROGRAM_STATE_STOPPING:
            program_data.state = PROGRAM_STATE_IDLE;

            status.type = PROGRAM_STATUS_TYPE_SERIES_STOPPED;
            status.data.series_stopped.program_id = program_data.program.id;
            status.data.series_stopped.series_index = serie_index(&program_data.program, serie);
            status.data.series_stopped.event_index = event_index(serie, event);
            call_status_callbacks(&status);

            status.type = PROGRAM_STATUS_TYPE_SERIES_NEXT;
            status.data.series_next.program_id = program_data.program.id;
            status.data.series_next.series_index = serie_index(&program_data.program, serie);
            call_status_callbacks(&status);

            send_status_chrono(0, NULL);

            break;
    }
}

static error_code_t json_validate(const void* buffer, uint32_t length)
{
  JSONStatus_t  status;

  status = JSON_Validate(buffer, length);
  if (status != JSONSuccess)
  {
    return ERROR_CODE_PROGRAM_JSON_INVALID;
  }

  return ERROR_CODE_OK;
}

static error_code_t json_find_tag(json_value_t* value, const char* tag, const void* buffer, uint32_t length)
{
  JSONStatus_t  status;

  status = JSON_SearchConst(buffer, length, tag, strlen(tag), &value->value, &value->length, &value->type);
  if (status != JSONSuccess)
  {
    return ERROR_CODE_PROGRAM_JSON_TAG_NOT_FOUND;
  }

  return ERROR_CODE_OK;
}

static void json_iterator_init(json_iterator_t* iterator, const void* buffer, uint32_t length)
{
    iterator->buffer = buffer;
    iterator->length = length;
    iterator->start = 0;
    iterator->next = 0;
}

static error_code_t json_get_next(json_iterator_t* iterator)
{
  JSONStatus_t  status;

  status = JSON_Iterate(iterator->buffer, iterator->length, &iterator->start, &iterator->next, &iterator->pair);
  if (status != JSONSuccess)
  {
      return ERROR_CODE_PROGRAM_JSON_ITERATOR_FAILED;
  }

  return ERROR_CODE_OK;
}

#if 0
static void print_pair(JSONPair_t* pair)
{
      static char* json_types[] =
      {
          "invalid",
          "string",
          "number",
          "true",
          "false",
          "null",
          "object",
          "array"
      };

      if (pair->keyLength == 0)
      {
          printf("found (%s):  - %.*s\n", json_types[pair->jsonType], (int)pair->valueLength, pair->value);
      }
      else
      {
          printf("found (%s): %.*s - %.*s\n", json_types[pair->jsonType], (int)pair->keyLength, pair->key, (int)pair->valueLength, pair->value);
      }
}
#endif

#if 0
static void print_program(const program_t* program)
{
    int     i;
    int     j;
    int     k;

    const serie_t*  serie;
    const event_t*  event;
    const audio_t*  audio;

    printf("Program:\n");
    for (i = 0; i < program->count; i++)
    {
        serie = &program->serie[i];

        printf("  Serie %d\n", i);
        printf("    Optional: %s\n", serie->optional ? "True" : "False");


        for (j = 0; j < serie->count; j++)
        {
            event = &serie->event[j];

            printf("    Event %d\n", j);
            printf("      Duration: %d\n", event->duration);
            switch (event->command)
            {
                case COMMAND_SHOW:      printf("      Command: Show\n");    break;
                case COMMAND_HIDE:      printf("      Command: Hide\n");    break;
                case COMMAND_NONE:                                          break;
            }
            switch (event->bank)
            {
                case TARGETS_BANK_A:    printf("      Bank: A\n");    break;
                case TARGETS_BANK_B:    printf("      Bank: B\n");    break;
                case TARGETS_BANK_ALL:  printf("      Bank: All\n");  break;
            }

            audio = &event->audio;
            if (audio->count > 0)
            {
                printf("      Audio:\n");
                for (k = 0; k < audio->count; k++)
                {
                    printf("        ID: %d\n", audio->id[k]);
                }
            }
        }
    }
}
#endif

static error_code_t parse_audio(audio_t* audio, const void* buffer, uint32_t length)
{
    error_code_t        res;
    uint32_t            count;
    json_iterator_t     audio_ids;
    audio_id_t*         id;
    char                string[10];

    json_iterator_init(&audio_ids, buffer, length);
    count = 0;

    do {
        id = &audio->id[count];

        res = json_get_next(&audio_ids);
        if (res != ERROR_CODE_OK)
        {
            break;
        }

        snprintf(string, MIN(sizeof(string), audio_ids.pair.valueLength + 1), "%s", audio_ids.pair.value);
        *id = strtol(string, NULL, 0);

        count++;
    } while (res == ERROR_CODE_OK);

    audio->count = count;

    return ERROR_CODE_OK;
}

static error_code_t parse_events(serie_t* serie, const void* buffer, uint32_t length)
{
    error_code_t        res;
    uint32_t            count;
    uint32_t            start_time;
    event_t*            event;
    json_iterator_t     events;
    json_value_t        duration;
    json_value_t        command;
    json_value_t        bank;
    json_value_t        audio_ids;
    char                string[10];

    json_iterator_init(&events, buffer, length);
    count = 0;
    start_time = 0;

    do {
        event = &serie->event[count];

        res = json_get_next(&events);
        if (res != ERROR_CODE_OK)
        {
            break;
        }

        res = json_find_tag(&duration, "duration", events.pair.value, events.pair.valueLength);
        if (res == ERROR_CODE_OK)
        {
            snprintf(string, MIN(sizeof(string), duration.length) + 1, "%s", duration.value);
            event->duration = strtol(string, NULL, 0);

            event->start_time = start_time;
            start_time += event->duration;
            serie->total_time += event->duration;

            res = json_find_tag(&command, "command", events.pair.value, events.pair.valueLength);
            if (res == ERROR_CODE_OK)
            {
                if (strncmp("show", command.value, command.length) == 0)
                {
                    event->command = COMMAND_SHOW;
                }
                else if (strncmp("hide", command.value, command.length) == 0)
                {
                    event->command = COMMAND_HIDE;
                }
                else
                {
                    event->command = COMMAND_NONE;
                }
            }
            else
            {
                event->command = COMMAND_NONE;
            }

            res = json_find_tag(&bank, "bank", events.pair.value, events.pair.valueLength);
            if (res == ERROR_CODE_OK)
            {
                if (strncmp("A", bank.value, bank.length) == 0)
                {
                    event->bank = TARGETS_BANK_A;
                }
                else if (strncmp("B", bank.value, bank.length) == 0)
                {
                    event->bank = TARGETS_BANK_B;
                }
                 else
                {
                    event->bank = TARGETS_BANK_NONE;
                }
            }
            else
            {
                event->bank = TARGETS_BANK_ALL;
            }

            res = json_find_tag(&audio_ids, "audio_ids", events.pair.value, events.pair.valueLength);
            if (res == ERROR_CODE_OK)
            {
                parse_audio(&event->audio, audio_ids.value, audio_ids.length);
            }

            count++;
            res = ERROR_CODE_OK;
        }
    } while (res == ERROR_CODE_OK);

    serie->count = count;

    return ERROR_CODE_OK;
}

static error_code_t parse_series(program_t* program, const void* buffer, uint32_t length)
{
    error_code_t        res;
    uint32_t            count;
    json_iterator_t     series;
    serie_t*            serie;
    json_value_t        optional;
    json_value_t        events;

    json_iterator_init(&series, buffer, length);
    count = 0;

    do {
        serie = &program->serie[count];

        res = json_get_next(&series);
        if (res == ERROR_CODE_OK)
        {
#if 0
            res = json_find_tag(&name, "name", series.pair.value, series.pair.valueLength);
            if (res != ERROR_CODE_OK)
            {
                break;
            }
            printf("found: %.*s\n", (int)name.length, name.value);
#endif

            res = json_find_tag(&optional, "optional", series.pair.value, series.pair.valueLength);
            if (res != ERROR_CODE_OK)
            {
                break;
            }

            if (strncmp("false", optional.value, optional.length) == 0)
            {
                serie->optional = false;
            }
            else
            {
                serie->optional = true;
            }

            res = json_find_tag(&events, "events", series.pair.value, series.pair.valueLength);
            if (res != ERROR_CODE_OK)
            {
                break;
            }

            serie->total_time = 0;
            parse_events(serie, events.value, events.length);

            count++;
            res = ERROR_CODE_OK;
        }
    } while(res == ERROR_CODE_OK);

    program->count = count;

    return ERROR_CODE_OK;
}

error_code_t program_load(program_id_t id)
{
  error_code_t  res;
  json_value_t  series;

  const void*   buffer;
  uint32_t      length;

  if (program_data.state != PROGRAM_STATE_IDLE)
  {
      return ERROR_CODE_PROGRAM_NOT_IDLE;
  }

  memset(&program_data.program, 0, sizeof(program_data.program));

  printf("Load %d\n", id);

  res = programs_get_by_id(id, &buffer, &length);
  if (res != ERROR_CODE_OK)
  {
      return res;
  }

  res = json_validate(buffer, strlen(buffer));
  if (res != ERROR_CODE_OK)
  {
    return res;
  }

  res = json_find_tag(&series, "series", buffer, strlen(buffer));
  if (res != ERROR_CODE_OK)
  {
    return res;
  }

  res = parse_series(&program_data.program, series.value, series.length);

  program_data.program.id = id;
  program_data.running.serie = &program_data.program.serie[0];
  program_data.running.event = &program_data.running.serie->event[0];

#if 0
  print_program(&program_data.program);
#endif

  return res;
}

error_code_t program_register_status_listener(program_status_callback_t callback, void* userdata)
{
    unsigned int        i;
    status_callback_t*  current;

    for (i = 0; i < ARRAY_SIZE(program_data.status_callbacks); i++)
    {
        current = &program_data.status_callbacks[i];

        if (current->callback == NULL)
        {
            current->callback = callback;
            current->userdata = userdata;

            return ERROR_CODE_OK;
        }
    }

    return ERROR_CODE_PROGRAM_LISTENERS_FULL;
}

error_code_t program_get_series_count(uint32_t* count)
{
  *count = program_data.program.count;

  return ERROR_CODE_OK;
}

error_code_t program_set_serie(uint32_t id)
{
  program_status_data_t   status;

  printf("Set serie %d\n", id);

  if (program_data.state != PROGRAM_STATE_IDLE)
  {
      return ERROR_CODE_PROGRAM_NOT_IDLE;
  }

  if (id >= program_data.program.count)
  {
    return ERROR_CODE_PROGRAM_INVALID_SERIE;
  }

  program_data.running.serie = &program_data.program.serie[id];
  program_data.running.event = &program_data.running.serie->event[0];

  status.type = PROGRAM_STATUS_TYPE_SERIES_NEXT;
  status.data.series_next.program_id = program_data.program.id;
  status.data.series_next.series_index = serie_index(&program_data.program, program_data.running.serie);
  call_status_callbacks(&status);

  return ERROR_CODE_OK;
}

error_code_t program_start(void)
{
    if (program_data.state != PROGRAM_STATE_IDLE)
    {
        return ERROR_CODE_PROGRAM_NOT_IDLE;
    }

    program_data.state = PROGRAM_STATE_STARTING;

    return ERROR_CODE_OK;
}

error_code_t program_stop(void)
{
  if (program_data.state != PROGRAM_STATE_IDLE)
  {
      program_data.state = PROGRAM_STATE_STOPPING;
  }

  return ERROR_CODE_OK;
}

error_code_t program_init(void)
{
  error_code_t  res;

  res = timer_register_callback(program_timer);
  if (res != ERROR_CODE_OK)
  {
      return res;
  }

  return ERROR_CODE_OK;
}

