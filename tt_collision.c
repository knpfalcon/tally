#include "tt_collision.h"

/* CHECKS FOR A SOLID TILE
   This takes a pixel XY position on the map,
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

/* BASIC AABB COLLISION DETECTION */
bool tile_collision(t_map *m, int al, int ar, int ab, int at, int bl, int br, int bb, int bt)
{
   if (al < br || ar > bl ||  ab > bt ||  at < bb) return true;

   return false;
}

