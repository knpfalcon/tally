#ifndef TT_COLLISION_H
#define TT_COLLISION_H

#include "tt_map.h"
#include "tt_player.h"
#include "tt_main.h"
#include "tt_thing.h"

bool is_ground(t_map *m, int x, int y, t_player *p);

//NOTE: Only use this between player and things, or things and things
bool collision_check(void *a, void *b);

#endif // TT_COLLISION_H
