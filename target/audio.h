#ifndef AUDIO_H
#define AUDIO_H

#include "error_code.h"

#include <stdint.h>

typedef uint32_t audio_id_t;

typedef void (*audio_callback_t)(void* userdata);

typedef struct {
    audio_id_t      id;
    char            description[32];
} audio_info_t;

error_code_t audio_play(audio_id_t id, audio_callback_t callback, void* userdata);
error_code_t audio_stop(audio_callback_t callback, void* userdata);
error_code_t audio_get_info(uint32_t index, audio_info_t* info);
error_code_t audio_get_count(uint32_t* count);
error_code_t audio_init(void);

#endif // AUDIO_H
