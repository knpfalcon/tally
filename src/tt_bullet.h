#ifndef TT_BULLET_H
#define TT_BULLET_H

#include <stdbool.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include "tt_main.h"

#define MAX_SPARKS 8

typedef struct
{
   int start_x;
   int start_y;
   int end_x;
   int end_y;
   //bool draw;
} t_bullet;

typedef struct 
{
   int x;
   int y;
   bool active;
   unsigned char frame;
} t_bullet_sparks;

void animate_bullet_sparks(t_bullet_sparks *spark);
void draw_bullet_spark(t_bullet_sparks *spark, ALLEGRO_BITMAP *src_bmp, t_cam *cam);
void activate_bullet_spark(t_bullet_sparks *spark, int x, int y);
#endif // TT_BULLET_H
