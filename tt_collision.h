#ifndef TT_COLLISION_H
#define TT_COLLISION_H

#include "tt_map.h"
#include "tt_player.h"
#include "tt_main.h"

bool is_ground(t_map *m, int x, int y);
bool collision(t_map *m, int al, int ar, int ab, int at, int bl, int br, int bb, int bt);
#endif // TT_COLLISION_H
