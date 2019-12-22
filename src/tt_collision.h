#ifndef TT_COLLISION_H
#define TT_COLLISION_H

#include "tt_map.h"
#include "tt_player.h"
#include "tt_main.h"

bool is_ground(t_map *m, int x, int y, t_player *p);
bool check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

#endif // TT_COLLISION_H
