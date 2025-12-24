#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include "audio.h"

#include "error_code.h"

error_code_t audioplayer_run(audio_id_t id[], uint8_t count);
error_code_t audioplayer_init(void);

#endif // AUDIOPLAYER_H
