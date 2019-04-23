#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>


#include "memwatch/memwatch.h"

#include "sp_main.h"
#include "sp_map.h"

t_map *create_empty_map()
{
   t_map *m = NULL; //Pointer to return at the end of function.

   m = malloc(sizeof(t_map)); //Allocate memory for map
   if (m == NULL)
   {
      jlog("In file %s, Line %d. Couldn't create an empty map! m returns NULL." __FILE__, __LINE__);
      return NULL; //Return NULL on failure
   }

   for (int i = 0; i < 32; i++)
   {
      m->name[i] = 0;
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

bool save_map(t_map *m, ALLEGRO_DISPLAY *display)
{
   ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
   al_append_path_component(path, "data/maps");
   al_set_path_filename(path, (const char *)"map.spl");
   const char *filename = al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP);

   ALLEGRO_FILECHOOSER *file_dialog = al_create_native_file_dialog(filename, "Save Map", "*.spl", ALLEGRO_FILECHOOSER_SAVE);
   if(!file_dialog) return false;

   if(!al_show_native_file_dialog(display, file_dialog)) {
      if(file_dialog) al_destroy_native_file_dialog(file_dialog);
      jlog("Save File dialog closed.");
      return false;
   }

   filename = al_get_native_file_dialog_path(file_dialog, 0);

   al_destroy_native_file_dialog(file_dialog);

   ALLEGRO_FILE *fp = NULL;

   fp = al_fopen(filename, "wb");
   if (fp == NULL)
   {
      jlog("Error opening file to save map to!");
      return false;
   }

   al_fwrite(fp, m, sizeof(t_map));
   al_fwrite(fp, m->position, (MAP_WIDTH * MAP_HEIGHT) * sizeof(t_map_pos));

   al_fclose(fp);

   jlog("Map Saved.");
   return true;
}

t_map *load_map(const char *fname)
{
   t_map *m;

   ALLEGRO_FILE *fp = NULL;

   fp = al_fopen(fname, "rb");
   if (fp == NULL)
   {
      jlog("Error opening map file!");
      return NULL;
   }

   m = malloc(sizeof(t_map)); //Allocate memory for map
   if (m == NULL)
   {
      jlog("In file %s, Line %d. Couldn't create an empty map! m returns NULL." __FILE__, __LINE__);
      al_fclose(fp);
      return NULL; //Return NULL on failure
   }
   al_fread(fp, m, sizeof(t_map));

   m->position = malloc(MAP_WIDTH * MAP_HEIGHT * sizeof(t_map_pos));
   if (m->position == NULL)
   {
      free(m);
      al_fclose(fp);
      jlog("In file %s, Line %d. Couldn't create an empty map! m->position returns NULL." __FILE__, __LINE__);
      return NULL;
   }
   al_fread(fp, m->position, (MAP_WIDTH * MAP_HEIGHT) *sizeof(t_map_pos));

   jlog("Map Opened.");
   al_fclose(fp);
   return m;

}

void draw_map(ALLEGRO_BITMAP *d_bmp, ALLEGRO_BITMAP *tiles[], ALLEGRO_BITMAP *background, t_cam *c, t_map *m)
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
               al_draw_bitmap(tiles[m->position[x + y * MAP_WIDTH].tile], (x * TILE_SIZE) - c->x, (y * TILE_SIZE) - c->y, 0);
               //al_draw_bitmap_region(tile_sheet, m->position[x + y * MAP_WIDTH].tile_x * TILE_SIZE, m->position[x + y * MAP_WIDTH].tile_y * TILE_SIZE, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE) - c->x, (y * TILE_SIZE) - c->y, 0); //Draw the tile, subtracting the camera position
            }
         }
      }
   }
}
