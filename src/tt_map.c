#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>

#include "tt_main.h"
#include "tt_map.h"
#include "tt_thing.h"

/************************************************
 * Creates an empty map in memory               *
 ************************************************/
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
   strcpy(m->name, "EMPTY MAP");
   m->bg = 0;
   m->num_things = 0;
   m->player_start_x = 0;
   m->player_start_y = 0;

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

/************************************************
 * Frees a map from memory                      *
 ************************************************/
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

/************************************************
 * Saves a map to disk                          *
 ************************************************/
bool save_map(t_map *m, ALLEGRO_DISPLAY *display)
{
   ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
   al_append_path_component(path, "data/maps");
   //al_set_path_filename(path, (const char *)"*.spl");
   const char *filename = al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP);

   ALLEGRO_FILECHOOSER *file_dialog = al_create_native_file_dialog(filename,
                                                                   "Save Map",
                                                                   "*.spl",
                                                                   ALLEGRO_FILECHOOSER_SAVE);
   if(!file_dialog) return false;

   if(!al_show_native_file_dialog(display, file_dialog)) {
      if(file_dialog) al_destroy_native_file_dialog(file_dialog);
      jlog("Save File dialog closed.");
      return false;
   }

   if (al_get_native_file_dialog_count(file_dialog) == 0)
   {
      al_destroy_native_file_dialog(file_dialog);
      return false;
   }

   filename = al_get_native_file_dialog_path(file_dialog, 0);

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

   al_destroy_native_file_dialog(file_dialog);
   
   return true;
}

/************************************************
 * Loads a map from disk                        *
 ************************************************/
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

   m = malloc(sizeof(*m)); //Allocate memory for map
   if (m == NULL)
   {
      jlog("In file %s, Line %d. Couldn't create an empty map! m returns NULL." __FILE__, __LINE__);
      al_fclose(fp);
      return NULL; //Return NULL on failure
   }
   al_fread(fp, m, sizeof(*m));

   m->position = malloc( (MAP_WIDTH * MAP_HEIGHT) * sizeof(t_map_pos));
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

/************************************************
 * Draws only seen tiles/bg to view port        *
 ************************************************/
void draw_map(ALLEGRO_BITMAP *d_bmp, ALLEGRO_BITMAP *tile_sheet, ALLEGRO_BITMAP *item_sheet, ALLEGRO_BITMAP *background, t_cam *c, t_map *m, unsigned char *item_frame)
{
   al_set_target_bitmap(d_bmp);

   /**** BACKGROUND ****/
   #ifdef EDITOR
   al_clear_to_color(al_map_rgb(0,0,0));
   #endif
   al_draw_bitmap(background, 0, 0, 0);

   /**** TILES ****/
   //Which tiles are in view?
   int y_tiles_in_view = VIEWPORT_HEIGHT / TILE_SIZE + 1;   //Should be 10 tiles +1 row off view
   int x_tiles_in_view = VIEWPORT_WIDTH / TILE_SIZE + 1;    //Should be 13 tiles +1 Column off view
   int sx = c->x / TILE_SIZE;    //This is the tile to draw from, according to camera position
   int sy = c->y / TILE_SIZE;    //This is the tile to draw from, according to camera position
   al_hold_bitmap_drawing(true);
   for (int y = sy; y < sy + y_tiles_in_view; y++)
   {
      for (int x = sx; x < sx + x_tiles_in_view; x++)
      {
         if ((x < MAP_WIDTH) && (x > -1) && (y < MAP_HEIGHT) && (y > -1))
         {
            //TILE
            if (m->position[x + y * MAP_WIDTH].empty_tile == false)
            {
               //Draw the tile, subtracting the camera position
               al_draw_bitmap_region(tile_sheet,
                                     convert_index_to_pixel_xy(m->position[x + y * MAP_WIDTH].tile, 16, TILE_SIZE, RETURN_X),
                                     convert_index_to_pixel_xy(m->position[x + y * MAP_WIDTH].tile, 16, TILE_SIZE, RETURN_Y),
                                     TILE_SIZE,
                                     TILE_SIZE,
                                     (x * TILE_SIZE) - c->x,
                                     (y * TILE_SIZE) - c->y,
                                     0);
               
               //Draw animated tiles
               if (m->position[x + y * MAP_WIDTH].tile % 2 == 0 &&  m->position[x + y * MAP_WIDTH].tile >= 240)
               {
                  al_draw_bitmap_region(tile_sheet,
                                     convert_index_to_pixel_xy(m->position[x + y * MAP_WIDTH].tile, 16, TILE_SIZE, RETURN_X) + (*item_frame * TILE_SIZE),
                                     convert_index_to_pixel_xy(m->position[x + y * MAP_WIDTH].tile, 16, TILE_SIZE, RETURN_Y),
                                     TILE_SIZE,
                                     TILE_SIZE,
                                     (x * TILE_SIZE) - c->x,
                                     (y * TILE_SIZE) - c->y,
                                     0);
               }
               else if (m->position[x + y * MAP_WIDTH].tile % 2 != 0 &&  m->position[x + y * MAP_WIDTH].tile >= 240)
               {
                  al_draw_bitmap_region(tile_sheet,
                                     convert_index_to_pixel_xy(m->position[x + y * MAP_WIDTH].tile, 16, TILE_SIZE, RETURN_X),
                                     convert_index_to_pixel_xy(m->position[x + y * MAP_WIDTH].tile, 16, TILE_SIZE, RETURN_Y),
                                     TILE_SIZE,
                                     TILE_SIZE,
                                     (x * TILE_SIZE) - c->x,
                                     (y * TILE_SIZE) - c->y,
                                     0);
               }
               
            }
            
            //ITEMS
            if (m->position[x + y * MAP_WIDTH].item > 0)
            {
               al_draw_bitmap_region(item_sheet,
                                     (m->position[x + y * MAP_WIDTH].item -1) * TILE_SIZE,
                                     *item_frame * TILE_SIZE,
                                     TILE_SIZE,
                                     TILE_SIZE,
                                     (x * TILE_SIZE) - c->x,
                                     (y * TILE_SIZE) - c->y,
                                     0);
            }

         }
      }
   }
   al_hold_bitmap_drawing(false);
}

/************************************************
 * Get a position on the map easily             *
 ************************************************/
t_map_pos *get_map_position(t_map *m, int x, int y)
{
   x /= TILE_SIZE;
   y /= TILE_SIZE;

   if (m == NULL) return NULL;
	if (x < 0 || x >= MAP_WIDTH) return NULL;
	if (y < 0 || y >= MAP_HEIGHT) return NULL;

   return &m->position[x + y * MAP_WIDTH];
}

void check_doors(t_player *player, t_map *m)
{
   for (int y = 0; y < MAP_HEIGHT; y++)
   {
      for (int x = 0; x < MAP_WIDTH; x++)
      {
         if (m->position[ x + y * MAP_WIDTH].tile == 127)
         {
            m->position[ x + y * MAP_WIDTH].empty_tile = true;
         }
      }
   }
}