#ifndef SP_PLAYER_H
#define SP_PLAYER_H

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
   int state;
   ALLEGRO_TIMER *timer;
   float speed;
} t_player;

enum player_state {GROUNDED = 0, WALKING = 1, JUMPING = 2, FALLING = 3};

void draw_player(ALLEGRO_BITMAP *bmp, t_cam *c, t_player *p);
void check_player_animation(t_player *p);
#endif // SP_PLAYER_H
