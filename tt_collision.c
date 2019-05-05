#include "tt_collision.h"
#include <allegro5/allegro_primitives.h>

/*************************************************
 * Checks for a solid tile at X/Y                *
 *************************************************/
/* This takes a pixel XY position on the map,
   and returns true if the tile there is
   solid. All tiles at indexes 128 through 255
   on the tile sheet are background tiles and
   are not solid. */
bool is_ground(t_map *m, int x, int y)
{
   x /= TILE_SIZE;
   y /= TILE_SIZE;

   if (m->position[x + y * MAP_WIDTH].empty_tile == true ||
       m->position[x + y * MAP_WIDTH].tile >= 128)
   {
         return false;
   }

   return true;
}

/*************************************************
 * Basic Bounding Box collision detection        *
 *************************************************/
//bool collision(t_map *m, int al, int ar, int ab, int at, int bl, int br, int bb, int bt)
//{
//   if (al < br || ar > bl ||  ab > bt ||  at < bb) return true;
//
//   return false;
//}
bool check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{


   if ( ((x1)>(x2+w2)) || ((x1+w1)<=(x2)) ||
        ((y1)>(y2+h2)) || ((y1+h1)<=(y2))  ) return false;

   return true;
}
#ifdef DEBUG
void draw_bb(t_cam *c, int x1, int y1, int w1, int h1)
{
   al_draw_rectangle(x1 - c->x, y1 - c->y, (x1 + w1) - c->x, (y1 + h1) - c->y, al_map_rgb(170,0,0), 1);
}
#endif // DEBUG


