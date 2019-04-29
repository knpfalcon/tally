#ifndef TT_PLAYER_H
#define TT_PLAYER_H

#include <allegro5/allegro.h>

#include "tt_main.h"

typedef struct t_player
{
   ALLEGRO_BITMAP *bitmap;
   ALLEGRO_BITMAP *frame[8];
   int x;
   int y;
   unsigned char cur_frame;
   unsigned char direction;
   unsigned char state;
   ALLEGRO_TIMER *timer;
   float speed;
   int vel_x;
   int vel_y;
   int right;
   int left;
   int top;
   int bottom;
   bool on_ground;
   bool jump_pressed;
   bool jumping;
   unsigned char health;
   unsigned int score;
   #ifdef DEBUG
   int x1, x2, x3;
   #endif // DEBUG
} t_player;

enum player_state {STOPPED = 0, WALKING = 1, JUMPING = 2, FALLING = 3};
enum direction {RIGHT = 0, LEFT = 1};

void draw_player(ALLEGRO_BITMAP *bmp, t_cam *c, t_player *p, unsigned char direction);
void animate_player(t_player *p);
void show_player_hotspot(ALLEGRO_BITMAP *bmp, t_cam *c, t_player *p);
#endif // SP_PLAYER_H
