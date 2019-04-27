#ifndef SP_PLAYER_H
#define SP_PLAYER_H

#include <allegro5/allegro.h>

typedef struct t_player
{
   int x;
   int y;
   ALLEGRO_BITMAP *bitmap;
   ALLEGRO_BITMAP *frame[8];
   unsigned char cur_frame;
} t_player;

#endif // SP_PLAYER_H
