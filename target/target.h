#ifndef TARGET_H
#define TARGET_H

#include <error_code.h>

error_code_t target_init(void);
void target_fatal(error_code_t error);

#endif // TARGET_H
