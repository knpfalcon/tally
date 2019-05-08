#ifndef TT_BULLET_H
#define TT_BULLET_H

#include <stdbool.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>

typedef struct t_bullet
{
   ALLEGRO_BITMAP *bitmap;
   int x;
   int y;
   bool hurt_player;
   int bb_top;
   int bb_left;
   int bb_width;
   int bb_height;
} t_bullet;


#endif // TT_BULLET_H
