#ifndef TT_BULLET_H
#define TT_BULLET_H

#include <stdbool.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>

typedef struct t_bullet
{
   int start_x;
   int start_y;
   int end_x;
   int end_y;
   bool draw;
} t_bullet;


#endif // TT_BULLET_H
