#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>

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

   for (int y = 0; y < MAP_HEIGHT; y++)
   {
      for (int x = 0; x < MAP_WIDTH; x++)
      {
         m->position[x + y * MAP_WIDTH].empty_tile = true;
      }
   }

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


void draw_map(ALLEGRO_BITMAP *d_bmp, ALLEGRO_BITMAP *tile_sheet, ALLEGRO_BITMAP *background, t_cam *c, t_map *m)
{
   al_set_target_bitmap(d_bmp);
   al_draw_bitmap(background, 0, 0, 0);
   //al_clear_to_color(al_map_rgba(0, 0, 0, 0));
                                                            //Which tiles are in view? The view port size divided by the tile size plus 1.
   int y_tiles_in_view = VIEWPORT_HEIGHT / TILE_SIZE + 1;   //Should be 10 tiles +1 row off view
   int x_tiles_in_view = VIEWPORT_WIDTH / TILE_SIZE + 1;    //Should be 13 tiles +1 Column off view


   int sx = c->x / TILE_SIZE;      //This is the tile to draw from according to camera position
   int sy = c->y / TILE_SIZE;      //This is the tile to draw from according to camera position

   for (int y = sy; y < sy + y_tiles_in_view; y++)
   {
      for (int x = sx; x < sx + x_tiles_in_view; x++)
      {
         if ((x < MAP_WIDTH) && (x > -1) && (y < MAP_HEIGHT) && (y > -1))
         {
            if (m->position[x + y * MAP_WIDTH].empty_tile == false)
            {
               al_draw_bitmap_region(tile_sheet, m->position[x + y * MAP_WIDTH].tile_x * TILE_SIZE, m->position[x + y * MAP_WIDTH].tile_y * TILE_SIZE, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE) - c->x, (y * TILE_SIZE) - c->y, 0); //Draw the tile, subtracting the camera position
            }
         }
      }
   }
}
