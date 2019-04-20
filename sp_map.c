#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sp_main.h"
#include "sp_map.h"


t_map *create_empty_map()
{
   t_map *m; //Pointer to return at the end of function.

   m = malloc(sizeof(t_map)); //Allocate memory for map
   if (m == NULL)
   {
      jlog("In file %s, Line %d. Couldn't create an empty map! m returns NULL." __FILE__, __LINE__);
      return NULL; //Return NULL on failure
   }

   strcpy_s(m->name, 32, "EMPTY MAP");

   m->position = malloc(MAP_WIDTH * MAP_HEIGHT * sizeof(t_map_pos));
   if (m->position == NULL)
   {
      free(m);
      jlog("In file %s, Line %d. Couldn't create an empty map! m->position returns NULL." __FILE__, __LINE__);
      return NULL;
   }

   memset(m->position, 0, MAP_WIDTH * MAP_HEIGHT * sizeof(t_map_pos));

   jlog("Empty map data allocated. Returning map pointer (m).");
   return m;
}

void destroy_map(t_map *m)
{
   if (m == NULL)
   {
      jlog("Map was NULL. Nothing to destroy.");
      return;
   }

   if (m->position != NULL)
   {
      free(m->position);
   }

   free(m);

   jlog("Map Destroyed.");
}
