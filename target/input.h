#ifndef INPUT_H
#define INPUT_H

#include "error_code.h"

typedef enum {
  INPUT_EVENT_UP,
  INPUT_EVENT_DOWN,
  INPUT_EVENT_LEFT,
  INPUT_EVENT_RIGHT,
} input_event_t;

typedef void (*input_callback_t)(input_event_t event);

error_code_t input_register_callback(input_callback_t callback);

#endif // INPUT_H
