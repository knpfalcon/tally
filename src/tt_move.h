#ifndef TT_MOVE_H
#define TT_MOVE_H

#include "tt_map.h"
#include "tt_thing.h"
#include "tt_collision.h"
#include "tt_sound.h"

void check_horizontal_tile_collision(t_map *map, void *thing, int x1, int x2, int old_x);
bool return_horizontal_tile_collision(t_map *map, void *thing, int x1, int x2);
void check_ceiling(t_map *map, void *thing, int x1, int x2);
void check_floor(t_map *map, void *thing, int x1, int x2);
void apply_gravity(void *thing, int force);
void jump(void *thing, int strength);

#endif