#ifndef SCENE_H
#define SCENE_H

#include "error_code.h"

int scene_get_count(void);
char* scene_get_scene_name(int index);
error_code_t scene_init(void);

#endif // SCENE_H
