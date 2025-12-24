#ifndef FILES_H
#define FILES_H

#include "error_code.h"

#include <stdint.h>

error_code_t files_read(void* buffer, uint32_t* length);
error_code_t files_get_size(uint32_t* size);
error_code_t files_open(const char* filename);
error_code_t files_close(void);

error_code_t files_init(void);

#endif // FILES_H
