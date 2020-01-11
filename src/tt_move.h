#ifndef TT_MOVE_H
#define TT_MOVE_H

#include "tt_map.h"
#include "tt_thing.h"
#include "tt_collision.h"
#include "tt_sound.h"

void check_horizontal_tile_collision(t_map *map, void *thing, int x1, int x2, int old_x); //This stops an object at a wall
bool return_horizontal_tile_collision(t_map *map, void *thing, int x1, int x2); //If there's a wall this returns true
void check_ceiling(t_map *map, void *thing, int x1, int x2); //Checks ceiling collision
void check_floor(t_map *map, void *thing, int x1, int x2, int height); //Checks floor collision
void apply_gravity(void *thing, int force); //Applies gravity force
void jump(void *thing, int strength); //Makes something jump

#endif