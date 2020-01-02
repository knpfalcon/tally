#include <stdio.h>
#include <stdbool.h>

#include "tt_collision.h"

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
   if (y < 0 || y + 1> MAP_HEIGHT * TILE_SIZE) return false;
   else
   {
      x /= TILE_SIZE;
      y /= TILE_SIZE;

      if (m->position[x + y * MAP_WIDTH].empty_tile == true ||
         m->position[x + y * MAP_WIDTH].tile >= 128)
      {
            return false;
      }
   }
   return true;
}

/*************************************************
 * Basic Bounding Box collision detection        *
 *************************************************/
bool check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) //Scope for this function is limited to this module
{


   if ( ((x1)>(x2+w2)) || ((x1+w1)<=(x2)) ||
        ((y1)>(y2+h2)) || ((y1+h1)<=(y2))  ) return false;

   return true;
}

bool collision_check(void *a, void *b)
{
   //Cast a and b as t_things. This works if you put a t_player type, becasue the first part of the structs are the same.
   if (check_collision( ((t_thing*)a)->x + ((t_thing*)a)->bb_left, ((t_thing*)a)->y + ((t_thing*)a)->bb_top, ((t_thing*)a)->bb_width, ((t_thing*)a)->bb_height, ((t_thing*)b)->x + ((t_thing*)b)->bb_left, ((t_thing*)b)->y + ((t_thing*)b)->bb_top, ((t_thing*)b)->bb_width, ((t_thing*)b)->bb_height))
   return true;
   
   return false;
}