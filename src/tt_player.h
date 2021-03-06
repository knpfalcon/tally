#ifndef TT_PLAYER_H
#define TT_PLAYER_H

#include <allegro5/allegro.h>

#include "tt_main.h"

#define PLAYER_HURT_TIME 64

typedef struct t_player
{
   ALLEGRO_BITMAP *bitmap;
   ALLEGRO_BITMAP *frame[8];
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
   bool jump_pressed;
   unsigned char health;
   unsigned int score;
   bool has_green_key;
} t_player;

enum player_state {STOPPED = 0, WALKING = 1, JUMPING = 2, FALLING = 3};
enum direction {RIGHT = 0, LEFT = 1};

void draw_player(ALLEGRO_BITMAP *bmp, t_cam *c, t_player *p, unsigned char direction);
void animate_player(t_player *p, int *speed);
void show_player_hotspot(ALLEGRO_BITMAP *bmp, t_cam *c, t_player *p);
#endif // SP_PLAYER_H
