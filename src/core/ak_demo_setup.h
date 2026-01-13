#ifndef AK_DEMO_SETUP_H
#define AK_DEMO_SETUP_H

#include "ak_physics.h"

typedef struct {
  ak_fixed_t width;
  ak_fixed_t height;
} ak_demo_config_t;

#ifdef __cplusplus
extern "C" {
#endif

void ak_demo_create_standard_scene(ak_world_t *world, ak_demo_config_t config);

#ifdef __cplusplus
}
#endif

#endif // AK_DEMO_SETUP_H
