#ifndef TT_COLLISION_H
#define TT_COLLISION_H

#include "tt_map.h"
#include "tt_player.h"
#include "tt_main.h"

bool is_ground(t_map *m, int x, int y);
//bool collision(t_map *m, int al, int ar, int ab, int at, int bl, int br, int bb, int bt);
bool check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

#ifdef DEBUG
void draw_bb(t_cam *c, int x1, int y1, int w1, int h1);
#endif // DEBUG

#endif // TT_COLLISION_H
