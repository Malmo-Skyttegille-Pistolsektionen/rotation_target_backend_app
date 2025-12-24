#ifndef TARGETS_H
#define TARGETS_H

#include <error_code.h>

typedef enum {
  TARGETS_BANK_NONE = 0x00,
  TARGETS_BANK_A    = 0x01,
  TARGETS_BANK_B    = 0x02,
  TARGETS_BANK_ALL  = 0xff
} targets_bank_t;

error_code_t targets_show(targets_bank_t bank);
error_code_t targets_hide(targets_bank_t bank);
error_code_t targets_init(void);

#endif // TARGETS_H
