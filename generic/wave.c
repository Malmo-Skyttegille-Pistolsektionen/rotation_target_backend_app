#include "wave.h"

#include <stdint.h>
#include <string.h>

typedef struct __attribute__((packed)) {
  uint8_t     name[4];      // "RIFF"
  uint32_t    file_size;    // file size
  uint8_t     type[4];      // WAVE"
} riff_header_t;

typedef struct __attribute__((packed)) {
  uint8_t     name[4];      // "fmt "
  uint32_t    length;       // header length
  uint16_t    format_type;  // 1 - PCM
  uint16_t    channels;     // channels
  uint32_t    sample_rate;  // sample rate
  uint32_t    data_rate;
  uint16_t    data_size;
  uint16_t    bit_per_sample;
} format_header_t;

typedef struct __attribute__((packed)) {
  uint8_t     name[4];      // "data"
  uint32_t    size;         // size of data
  uint8_t     data[];
} data_header_t;

error_code_t wave_parse_header(wave_header_t* header, void* data, uint32_t size)
{
  riff_header_t*    riff_header;
  format_header_t*  format_header;
  data_header_t*    data_header;
  char*             next;

  riff_header = (riff_header_t*)data;

  if (strncmp((char*)riff_header->name, "RIFF", 4) != 0)
  {
    return ERROR_CODE_WAVE_INVALID_FORMAT;
  }

  if (strncmp((char*)riff_header->type, "WAVE", 4) != 0)
  {
    return ERROR_CODE_WAVE_INVALID_FORMAT;
  }

  next = (char*)&riff_header[1];

  while (next != NULL)
  {
    if (strncmp(next, "fmt ", 4) == 0)
    {
      format_header = (format_header_t*)next;
      header->channels = format_header->channels;
      header->sample_rate = format_header->sample_rate;
    }
    else if (strncmp(next, "data", 4) == 0)
    {
      data_header = (data_header_t*)next;
      header->pcm.size = data_header->size;
      header->pcm.offset = (uint32_t)((uintptr_t)data_header->data - (uintptr_t)data);
    }

    next = &next[*(uint32_t*)(&next[4]) + 8];
    if ((uintptr_t)next > (uintptr_t)data + size)
    {
      next = NULL;
    }
  }

  return ERROR_CODE_OK;
}

