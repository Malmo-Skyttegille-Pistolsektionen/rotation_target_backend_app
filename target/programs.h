#ifndef PROGRAMS_H
#define PROGRAMS_H

#include "error_code.h"

#include <stdint.h>

typedef uint8_t program_id_t;

typedef struct {
    program_id_t    id;
    char            title[32];
    char            description[32];
    uint32_t        size;
} program_info_t;

error_code_t programs_read(program_id_t id, void* buffer, uint32_t* length, uint32_t offset);
error_code_t programs_get_by_id(program_id_t id,  const void** buffer, uint32_t* length);
error_code_t programs_get_by_index(uint32_t index, program_id_t* id, const void** buffer, uint32_t* length);
error_code_t programs_get_size(program_id_t id,  uint32_t* size);
error_code_t programs_get_info(uint32_t index, program_info_t* info);
error_code_t programs_get_count(uint32_t* count);
error_code_t programs_init(void);

#endif // PROGRAMS_H
