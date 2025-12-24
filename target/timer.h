#ifndef TIMER_H
#define TIMER_H

#include "error_code.h"

typedef void (*timer_callback_t)(void);

error_code_t timer_register_callback(timer_callback_t);
error_code_t timer_init(void);

#endif // TIMER_H
