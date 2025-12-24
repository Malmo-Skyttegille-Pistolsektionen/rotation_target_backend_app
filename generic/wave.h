#ifndef WAVE_H
#define WAVE_H

#include "error_code.h"

#include <stdint.h>

typedef struct {
  uint32_t    channels;
  uint32_t    sample_rate;
  struct {
    uint32_t    offset;     // offset to pcm data
    uint32_t    size;       // bytes of pcm data
  }           pcm;
} wave_header_t;

error_code_t wave_parse_header(wave_header_t* header, void* data, uint32_t size);

#endif // WAVE_H
