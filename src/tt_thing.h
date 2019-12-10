#ifndef TT_ENEMY_H
#define TT_ENEMY_H

#include <allegro5/allegro5.h>
#include <stdlib.h>
#include "tt_map.h"
#include "tt_main.h"

#define MAX_THINGS 256
#define MAX_ENEMY_FRAMES 8
#define ENEMY_SPIKES 1

typedef struct
{
   ALLEGRO_BITMAP *bitmap;
   ALLEGRO_BITMAP *frame[MAX_ENEMY_FRAMES];
   int x;
   int y;
   unsigned char cur_frame;
   unsigned char direction;
   unsigned char state;
   unsigned char hurt;
   int vel_x;
   int vel_y;
   int bb_left;
   int bb_top;
   int bb_width;
   int bb_height;
   int muzzle_x;
   int muzzle_y;
   int muzzle_time;
   int shoot_time;
   bool draw;
   bool on_ground;
   unsigned char type;
   bool active;
} t_thing;

void draw_things(t_map *m, t_thing *t, t_cam *c, int count);
void clear_things(t_thing *t);
#endif