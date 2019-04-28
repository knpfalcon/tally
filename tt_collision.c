#include "memwatch/memwatch.h"

#include "tt_collision.h"

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

bool tile_collision(t_map *m, int al, int ar, int ab, int at, int bl, int br, int bb, int bt)
{
   if (al < br || ar > bl ||  ab > bt ||  at < bb) return true;

   return false;
}

