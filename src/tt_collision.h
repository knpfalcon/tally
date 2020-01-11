#ifndef TT_COLLISION_H
#define TT_COLLISION_H

#include "tt_map.h"
#include "tt_player.h"
#include "tt_main.h"
#include "tt_thing.h"

bool is_ground(t_map *m, int x, int y);
bool check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
//NOTE: Only use this between player and things, or things and things (Use with caution!)
bool collision_check(void *a, void *b);

#endif // TT_COLLISION_H
