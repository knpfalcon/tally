#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>

#include "tt_main.h"
#include "tt_map.h"
#include "tt_thing.h"
#include "tt_edit.h"
#include "tt_player.h"

const float FPS = 60;
const float ANIM_SPEED = 8;

const int DISPLAY_WIDTH = 640 * DISPLAY_MULTIPLIER;
const int DISPLAY_HEIGHT = 400 * DISPLAY_MULTIPLIER;

int item_selected = 1;
int thing_selected = 1;

t_cam cam;
t_mouse sp_mouse;
t_map *map = NULL;
t_player player;
t_thing thing[MAX_THINGS];
unsigned int enemy_count = 0;
unsigned char item_frame = 0;

t_conditional cond = {false, false, false, false};

//const char *filename = NULL;


//enum KEYS {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_Z, KEY_LSHIFT, KEY_N,KEY_PAD_PLUS, KEY_PAD_MINUS};
bool key[16] = {false};
enum KEYS {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_Z, KEY_LSHIFT, KEY_X, KEY_T, KEY_N, KEY_PAD_PLUS, KEY_PAD_MINUS, KEY_E, KEY_R, KEY_Q, KEY_F, KEY_G};

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *FPS_TIMER = NULL;
ALLEGRO_TIMER *ANIM_TIMER = NULL;
ALLEGRO_MOUSE_STATE mouse;

ALLEGRO_FONT *reg_font = NULL;

//Bitmaps that get loaded from disk
ALLEGRO_BITMAP *tile_sheet = NULL;
ALLEGRO_BITMAP *item_sheet = NULL;
ALLEGRO_BITMAP *bg = NULL;
ALLEGRO_BITMAP *stat_border = NULL;

//Bitmaps for enemies
ALLEGRO_BITMAP *b_enemy_spikes = NULL;
ALLEGRO_BITMAP *b_orlo = NULL;
ALLEGRO_BITMAP *b_orlo_frame_0 = NULL;
//ALLEGRO_BITMAP *player_start = NULL;

//Bitmaps that get drawn to
ALLEGRO_BITMAP *mini_map = NULL;
ALLEGRO_BITMAP *view_port = NULL;
ALLEGRO_BITMAP *game = NULL;


/*************************************************
 * Initiate everything the game needs at startup *
 *************************************************/
int init_game()
{
   //Initialize Allegro
   if(!al_init())
   {
      jlog("Failed to initialize allegro!\n");
      return -1;
   }
   uint32_t version = al_get_allegro_version();
   int major = version >> 24;
   int minor = (version >> 16) & 255;
   int revision = (version >> 8) & 255;
   int release = version & 255;
   jlog("Allegro %d.%d.%d release %d", major, minor, revision, release);

   event_queue = al_create_event_queue();
   if (!event_queue)
   {
      jlog("Failed to create event queue!");
      return -1;
   }
   jlog("Event queue created.");

   if (!al_init_image_addon())
   {
      jlog("Failed to initialize image_addon!");
      return -1;
   }
   jlog("Image add-on initialized.");

   FPS_TIMER = al_create_timer(1.0 / FPS);
   if(!FPS_TIMER)
   {
      jlog("Failed to create FPS timer!");
      return -1;
   }
   jlog("FPS timer created.");

   ANIM_TIMER = al_create_timer(1.0 / ANIM_SPEED);
   if(!ANIM_TIMER)
   {
      jlog("Failed to create Animation timer!");
      return -1;
   }
   jlog("Animation timer created.");

   if (!al_install_keyboard())
   {
      jlog("Failed to install keyboard!");
      return -1;
   }
   jlog("Keyboard installed.");

   if (!al_install_mouse())
   {
      jlog("Failed to install mouse!");
      return -1;
   }
   jlog("Mouse installed.");

   //Font Add-ons
   if(!al_init_font_addon())
   {
      jlog("Failed to install fonts addon!");
      return -1;
   }
   jlog("Fonts add-on installed.");

   if(!al_init_ttf_addon())
   {
      jlog("Failed to install ttf addon!");
      return -1;
   }
   jlog("TTF add-on installed.");

   //Primitives
   if (!al_init_primitives_addon())
   {
      jlog("Failed to install primitives addon!");
      return -1;
   }
   jlog("Primitives add-on initialized.");

   if (!al_init_native_dialog_addon())
   {
      jlog("Failed to initialize native dialog addon!");
   }
   jlog("Native dialog addon add-on initialized.");
   
   //Create Display
   //al_set_new_display_flags(ALLEGRO_OPENGL);
   al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
   display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
   if(!display)
   {
      jlog("Failed to create display!");
      return -1;
   }
   jlog("Display Created.");

   #ifdef DEBUG
   al_set_window_title(display, "Tedit - Tally Trauma Level Editor -- DEBUG");
   #endif // DEBUG

   #ifdef RELEASE
   al_set_window_title(display, "Tedit - Tally Trauma Level Editor");
   #endif // RELEASE

   stat_border = al_load_bitmap("data/status_border.png");
   if (stat_border == NULL)
   {
      jlog("Couldn't load status_border.png!");
      return -1;
   }

   bg = al_load_bitmap("data/bg_1.png");
   if (bg == NULL)
   {
      jlog("Couldn't load bg1.png!");
      return -1;
   }

   tile_sheet = al_load_bitmap("data/tile_sheet.png");
   if (tile_sheet == NULL)
   {
      jlog("Couldn't load tile_sheet.png!");
      return -1;
   }
   jlog("tile_sheet.png loaded.");

   item_sheet = al_load_bitmap("data/item_sheet.png");
   if (tile_sheet == NULL)
   {
      jlog("Couldn't load item_sheet.png!");
      return -1;
   }
   jlog("item_sheet.png loaded.");

   //Load enemy graphics (Skipping error checking)
   b_enemy_spikes = al_load_bitmap("data/spikes.png");
   b_orlo = al_load_bitmap("data/orlo.png");
   b_orlo_frame_0 = al_create_sub_bitmap(b_orlo, 0, 0, 32, 32);


   player.bitmap = al_load_bitmap("data/player.png");
   if (player.bitmap == NULL) { jlog("Couldn't load player.png"); return -1; }
   for (int j = 0; j < 8; j++)
   {
      player.frame[j] = al_create_sub_bitmap(player.bitmap, j * 32, 0, 32, 32);
      if (player.frame[j] == NULL)
      {
         jlog("Couldn't create sub-bitmap from player bitmap!");
         return -1;
      }
      jlog("Player frame %d created.", j);
   }

   player.cur_frame = 1;

   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
   mini_map = al_create_bitmap(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE);

   //Create the game bitmap that needs to be stretched to display
   game = al_create_bitmap(640, 400);

   reg_font = al_load_ttf_font(FONT_FILE, 8 * DISPLAY_MULTIPLIER, 0);
   if(!reg_font)
   {
      jlog("Failed to load %s", FONT_FILE);
      return -1;
   }
   jlog("%s loaded.", FONT_FILE);

   al_register_event_source(event_queue, al_get_display_event_source(display));
   al_register_event_source(event_queue, al_get_timer_event_source(FPS_TIMER));
   al_register_event_source(event_queue, al_get_timer_event_source(ANIM_TIMER));
   al_register_event_source(event_queue, al_get_keyboard_event_source());
   al_register_event_source(event_queue, al_get_mouse_event_source());

   al_clear_to_color(al_map_rgb(0, 0, 0));

   al_draw_text(reg_font, al_map_rgb(255,255,255), 16, 16 , 0, "Loading...");
   al_flip_display();

   al_set_system_mouse_cursor(display, ALLEGRO_SYSTEM_MOUSE_CURSOR_PRECISION);

   clear_things(thing);

   jlog("Game initialized.");
   return 0;
}

/************************************************
 * Draws the zoomed out map when LSHIFT is held *
 ************************************************/
void draw_mini_map()
{
   al_set_target_bitmap(mini_map);
   al_clear_to_color(al_map_rgb(0,0,0));
   al_hold_bitmap_drawing(true);
   for (int y = 0; y < MAP_HEIGHT; y++)
   {
      for (int x = 0; x < MAP_WIDTH; x++)
      {
         if (map->position[x + y * MAP_WIDTH].empty_tile == false)
         {
            al_draw_bitmap_region(tile_sheet,
                                  convert_index_to_pixel_xy(map->position[x + y * MAP_WIDTH].tile, 16, TILE_SIZE, RETURN_X),
                                  convert_index_to_pixel_xy(map->position[x + y * MAP_WIDTH].tile, 16, TILE_SIZE, RETURN_Y),
                                  TILE_SIZE,
                                  TILE_SIZE,
                                  (x * TILE_SIZE),
                                  (y * TILE_SIZE),
                                  0);
         }

      }
   }
   al_hold_bitmap_drawing(false);
   //al_draw_bitmap_region(player_start, 0, 0, 32, 32, map->player_start_x, map->player_start_y, 0);
   al_draw_bitmap(player.frame[player.cur_frame], player.x, player.y, 0);
}

/************************************************
 * Draws all the texts, info, and helpers       *
 ************************************************/
void show_info_stuff()
{
   if (cond.show_mini_map == false)
   {
      al_draw_bitmap(tile_sheet, TILE_SHEET_X * DISPLAY_MULTIPLIER, TILE_SHEET_Y * DISPLAY_MULTIPLIER, 0);
      al_draw_rectangle(convert_index_to_pixel_xy(sp_mouse.tile_selection, 16, TILE_SIZE, RETURN_X) + (TILE_SHEET_X * DISPLAY_MULTIPLIER),
                        convert_index_to_pixel_xy(sp_mouse.tile_selection, 16, TILE_SIZE, RETURN_Y) + (TILE_SHEET_Y * DISPLAY_MULTIPLIER),
                        convert_index_to_pixel_xy(sp_mouse.tile_selection, 16, TILE_SIZE, RETURN_X) + (TILE_SHEET_X * DISPLAY_MULTIPLIER) + TILE_SIZE,
                        convert_index_to_pixel_xy(sp_mouse.tile_selection, 16, TILE_SIZE, RETURN_Y) + (TILE_SHEET_Y * DISPLAY_MULTIPLIER) + TILE_SIZE,
                        al_map_rgb(255,255,0),
                        2);
      al_draw_textf(
                    reg_font,
                    al_map_rgb(255,255,255),
                    16 * DISPLAY_MULTIPLIER,
                    344 * DISPLAY_MULTIPLIER,
                    ALLEGRO_ALIGN_LEFT, "map->position %d, %d",
                    sp_mouse.over_tile_x,
                    sp_mouse.over_tile_y
                    );
      if (cond.name_map)
      {
         al_draw_filled_circle(16 * DISPLAY_MULTIPLIER - 2 * DISPLAY_MULTIPLIER, 4 * DISPLAY_MULTIPLIER,
                               2 * DISPLAY_MULTIPLIER,
                               al_map_rgb(255, 0, 0));
         al_draw_textf(reg_font,
                    al_map_rgb(255,255,255),
                    16 * DISPLAY_MULTIPLIER,
                    2 * DISPLAY_MULTIPLIER,
                    ALLEGRO_ALIGN_LEFT,
                    "%s*",
                    map->name);
      }
      if (!cond.name_map)
      {
         al_draw_textf(reg_font,
                     al_map_rgb(255,255,255),
                     16 * DISPLAY_MULTIPLIER,
                     2 * DISPLAY_MULTIPLIER,
                     ALLEGRO_ALIGN_LEFT,
                     "%s",
                     map->name);
      }
      al_draw_textf(reg_font,
                    al_map_rgb(255,255,255),
                    452 * DISPLAY_MULTIPLIER,
                    162 * DISPLAY_MULTIPLIER,
                    0,
                    "Tile Selected: %d",
                    sp_mouse.tile_selection);
      al_draw_textf(reg_font,
                    al_map_rgb(255,255,255),
                    452 * DISPLAY_MULTIPLIER,
                    178 * DISPLAY_MULTIPLIER,
                    0,
                    "Thing: %d    No. Things %d / %d",
                    thing_selected, map->num_things, MAX_THINGS);
      al_draw_textf(reg_font,
                    al_map_rgb(255,255,255),
                    452 * DISPLAY_MULTIPLIER,
                    228 * DISPLAY_MULTIPLIER,
                    0,
                    "Item: %d",
                    item_selected);
      if (cond.map_saved == true)
      {
         al_draw_text(reg_font,
                      al_map_rgb(0,255,0),
                      16 * DISPLAY_MULTIPLIER,
                      364 * DISPLAY_MULTIPLIER,
                      ALLEGRO_ALIGN_LEFT,
                      "MAP SAVED.");
      }
      else if (cond.map_saved == false)
      {
         al_draw_text(reg_font,
                      al_map_rgb(255,0,0),
                      16 * DISPLAY_MULTIPLIER,
                      364 * DISPLAY_MULTIPLIER,
                      ALLEGRO_ALIGN_LEFT,
                      "MAP NOT SAVED.");
      }
   }

}

/************************************************
 * Draws the player icon at starting position   *
 ************************************************/
void draw_player_start()
{
   al_set_target_bitmap(view_port);
   if ((cam.x - 32) < map->player_start_x  &&
       (cam.y - 32) < map->player_start_y  &&
       (cam.x + VIEWPORT_WIDTH) > map->player_start_x &&
       (cam.y + VIEWPORT_HEIGHT) >map->player_start_y)
   {
      al_draw_bitmap(player.frame[player.cur_frame], player.x - cam.x, player.y - cam.y, 0);
   }
}

/************************************************
 * The drawing function, called at redraw       *
 ************************************************/
void update_screen()
{
   draw_mini_map();

   if (cond.show_mini_map == false)
   {
      if (map != NULL)
      {
         draw_map(view_port, tile_sheet, item_sheet, bg, &cam, map, &item_frame);
         
         
         draw_things(map, thing, &cam, map->num_things);

         draw_player_start();
      }
      al_set_target_bitmap(game);
      al_clear_to_color(al_map_rgb(0,0,0));
      al_draw_bitmap(stat_border, 0, 0, 0);
      al_draw_bitmap(view_port, 16, 16, 0);

      /* Draws a zoomed in version of the currently selected tile
      if the screen is big enough. */
      if (DISPLAY_MULTIPLIER  >= 9)
      {
         al_draw_bitmap_region(tile_sheet,
                               convert_index_to_pixel_xy(sp_mouse.tile_selection, 16, TILE_SIZE, RETURN_X),
                               convert_index_to_pixel_xy(sp_mouse.tile_selection, 16, TILE_SIZE, RETURN_Y),
                               TILE_SIZE,
                               TILE_SIZE,
                               452,
                               32,
                               0);
      }
      al_draw_bitmap_region(item_sheet,
                               (item_selected -1) * TILE_SIZE,
                               0,
                               TILE_SIZE,
                               TILE_SIZE,
                               450,
                               236,
                               0);
      
      //Draw Selected enemy
      switch (thing_selected)
      {
      case ENEMY_SPIKES:
         al_draw_bitmap(b_enemy_spikes, 452, 188, 0);
         break;
      case ENEMY_BAD_ROBOT:
         al_draw_bitmap_region(b_orlo, 0, 0, 32, 32, 452, 188, 0);
         break;
         
      default:
         break;
      }
   }

   //Draw the zoomed out map
   al_set_target_bitmap(mini_map);
   if (cond.show_mini_map)
   {
      al_draw_rectangle(cam.x,
                        cam.y,
                        cam.x + VIEWPORT_WIDTH,
                        cam.y + VIEWPORT_HEIGHT,
                        al_map_rgb(255,255,255),
                        1 * DISPLAY_MULTIPLIER);
   }
   al_set_target_backbuffer(display);

   if (cond.show_mini_map == false) al_draw_scaled_bitmap(game, 0, 0, 640, 400, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);

   if (cond.show_mini_map)
   {
      al_draw_scaled_bitmap(mini_map, 0, 0, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT , 0);
   }

   show_info_stuff();
   al_flip_display();
}

/***************************************************
 * Clears all the things and checks for them again *
 **************************************************/
void check_map_for_things()
{  
   map->num_things = 0;
   clear_things(thing);
   for (int y = 0; y < MAP_HEIGHT; y++)
   {
      for (int x = 0; x < MAP_WIDTH; x++)
      {
         if (map->position[x + y * MAP_WIDTH].thing > 0 && map->num_things < MAX_THINGS)
         {
            map->num_things++;
            thing[map->num_things -1].active  = true;
            thing[map->num_things -1].x = x * 16;
            thing[map->num_things -1].y = y * 16;
         }

         if (map->position[x + y * MAP_WIDTH].thing == ENEMY_SPIKES)
         {
            thing[map->num_things -1].frame[0] = b_enemy_spikes;
            thing[map->num_things -1].bb_height = 16;
            thing[map->num_things -1].bb_width = 16;
         }
         if (map->position[x + y * MAP_WIDTH].thing == ENEMY_BAD_ROBOT)
         {
            thing[map->num_things -1].frame[0] = b_orlo_frame_0;
            
            thing[map->num_things -1].bb_height = 16;
            thing[map->num_things -1].bb_width = 16;
         }
      }
   }
}

/************************************************
 * Checks clicks inside the view port           *
 ************************************************/
void check_click_in_viewport()
{
   al_get_mouse_state(&mouse);
   if (cond.show_mini_map == false)
   {
      //If mouse is inside view port
      if (sp_mouse.x > VIEWPORT_X * DISPLAY_MULTIPLIER
         && sp_mouse.x < (VIEWPORT_WIDTH + VIEWPORT_X) * DISPLAY_MULTIPLIER
         && sp_mouse.y > VIEWPORT_Y * DISPLAY_MULTIPLIER
         && sp_mouse.y < (VIEWPORT_HEIGHT + VIEWPORT_Y) * DISPLAY_MULTIPLIER)
      {
         sp_mouse.over_tile_x = ((((sp_mouse.x - (16 * DISPLAY_MULTIPLIER)) + (cam.x * DISPLAY_MULTIPLIER)) / TILE_SIZE) / DISPLAY_MULTIPLIER);
         sp_mouse.over_tile_y = ((((sp_mouse.y - (16 * DISPLAY_MULTIPLIER)) + (cam.y * DISPLAY_MULTIPLIER)) / TILE_SIZE) / DISPLAY_MULTIPLIER);

         if (mouse.buttons & 1 && map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].empty_tile == true)
         {
            map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].empty_tile = false;
            map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].tile = sp_mouse.tile_selection;
            cond.map_saved = false;
         }
         else if (mouse.buttons & 2)
         {
            if (map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].empty_tile == false)
            {
               map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].empty_tile = true;
               cond.map_saved = false;
            }
         }
         else if (mouse.buttons & 4)
         {
            map->player_start_x = sp_mouse.over_tile_x * TILE_SIZE;
            map->player_start_y = sp_mouse.over_tile_y * TILE_SIZE;
            player.x = map->player_start_x;
            player.y = map->player_start_y;
            cond.map_saved = false;
         }
         else if (key[KEY_E]) //Add item
         {
            map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].item = item_selected;
            cond.map_saved = false;
         }
         else if (key[KEY_R]) //Remove item
         {
            if (map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].item > 0)
            {
               map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].item = 0;
               cond.map_saved = false;
            }
         }
   
         else if (key[KEY_F]) //Add thing
         {
            map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].thing = thing_selected;
            cond.map_saved = false;
            check_map_for_things();
         }

         else if (key[KEY_G]) //Remove thing
         {
            if (map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].thing > 0)
            {
               map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].thing = 0;
               cond.map_saved = false;
               check_map_for_things();
            }
            
         }
            
        
         else if (key[KEY_Q])
         {
            //Tile dropper
            al_set_system_mouse_cursor(display, ALLEGRO_SYSTEM_MOUSE_CURSOR_LINK);
            sp_mouse.tile_selection = map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].tile;
            
         }
      }
      else
      {
         sp_mouse.over_tile_x = 0;
         sp_mouse.over_tile_y = 0;
      }

      //Check mouse in Tile Selector
      if (sp_mouse.x > TILE_SHEET_X * DISPLAY_MULTIPLIER
         && sp_mouse.x < ( TILE_SHEET_WIDTH + (TILE_SHEET_X * DISPLAY_MULTIPLIER) )
         && sp_mouse.y > TILE_SHEET_Y * DISPLAY_MULTIPLIER
         && sp_mouse.y < ( TILE_SHEET_HEIGHT + (TILE_SHEET_Y * DISPLAY_MULTIPLIER) ) )
      {
         if (mouse.buttons & 1)
         {
               int x = (sp_mouse.x - (TILE_SHEET_X * DISPLAY_MULTIPLIER) ) / TILE_SIZE;
               int y = (sp_mouse.y - (TILE_SHEET_Y * DISPLAY_MULTIPLIER) )  / TILE_SIZE;
               int w = TILE_SHEET_WIDTH / TILE_SIZE;

               sp_mouse.tile_selection = x + y * w;
                  
         }
      }

   }
}

/************************************************
 * Makes sure the cam is within the map bounds  *
 ************************************************/
void check_cam_bounds() //Check to make sure camera is not out of bounds.
{
   if (cam.x < 0) cam.x = 0;
   if (cam.x > (MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH)
   {
      cam.x = (MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH;
   }

   if (cam.y < 0) cam.y = 0;
   if (cam.y > (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT)
   {
      cam.y = (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT;
   }

}

/************************************************
 * Changes tile selection with wheel or PAD +/- *
 ************************************************/
void check_tile_selection()
{
   if (mouse.z < sp_mouse.z || key[KEY_PAD_MINUS])
   {
      sp_mouse.z = mouse.z;
      if (sp_mouse.tile_selection > 0)
      {
         sp_mouse.tile_selection--;
      }
      else if (sp_mouse.tile_selection == 0)
      {
         sp_mouse.tile_selection = 255;
      }
   }
   if (mouse.z > sp_mouse.z || key[KEY_PAD_PLUS])
   {
      sp_mouse.z = mouse.z;

      if (sp_mouse.tile_selection < 255)
      {
         sp_mouse.tile_selection++;
      }
      else if (sp_mouse.tile_selection == 255)
      {
         sp_mouse.tile_selection = 0;
      }
   }
}

/************************************************
 * Opens a file dialog to open a new map        *
 ************************************************/
bool open_file_dialog()
{
   const char *filename = NULL;

   ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
   al_append_path_component(path, "data/maps");
   al_set_path_filename(path, NULL);
   filename = al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP);

   ALLEGRO_FILECHOOSER *file_dialog = al_create_native_file_dialog(filename, "Open Map", "*.spl", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
  
   if(!file_dialog) return false;
   
   if(!al_show_native_file_dialog(NULL, file_dialog)) {
      if (file_dialog) al_destroy_native_file_dialog(file_dialog);
      return false;
   }

   if (al_get_native_file_dialog_count(file_dialog) > 0)
   {
      filename = al_get_native_file_dialog_path(file_dialog, 0);

      destroy_map(map);
      map = NULL;
      map = load_map(filename);
      if (map == NULL)
      {
         jlog("Something went wrong with map loading!");
         destroy_map(map);
         map = NULL;
      }
      player.x = map->player_start_x;
      player.y = map->player_start_y;
      cam.x = player.x - VIEWPORT_WIDTH / 2 + 16;
      cam.y = player.y - VIEWPORT_HEIGHT / 2 + 16;
      check_map_for_things();
      cond.map_saved = true;
   }   
   
   if (file_dialog) al_destroy_native_file_dialog(file_dialog);

   return true;
}

/************************************************
 * Checks "Key Down" events                     *
 ************************************************/
void check_key_down(ALLEGRO_EVENT *ev)
{
   switch(ev->keyboard.keycode)
   {
      case ALLEGRO_KEY_UP:
         key[KEY_UP] = true;
         break;
      case ALLEGRO_KEY_DOWN:
         key[KEY_DOWN] = true;
         break;
      case ALLEGRO_KEY_LEFT:
         key[KEY_LEFT] = true;
         break;
      case ALLEGRO_KEY_RIGHT:
         key[KEY_RIGHT] = true;
         break;
      case ALLEGRO_KEY_W:
         key[KEY_UP] = true;
         break;
      case ALLEGRO_KEY_S:
         key[KEY_DOWN] = true;
         break;
      case ALLEGRO_KEY_A:
         key[KEY_LEFT] = true;
         break;
      case ALLEGRO_KEY_D:
         key[KEY_RIGHT] = true;
         break;
      case ALLEGRO_KEY_Z:
         key[KEY_Z] = true;
         break;
      case ALLEGRO_KEY_N:
         key[KEY_N] = true;
         break;
      case ALLEGRO_KEY_LSHIFT:
         key[KEY_LSHIFT] = true;
         if (cond.name_map == false)
         {
            cond.show_mini_map = true;
         }
         break;
      case ALLEGRO_KEY_PAD_PLUS:
         key[KEY_PAD_PLUS] = true;
         break;
      case ALLEGRO_KEY_F3:
         break;
      case ALLEGRO_KEY_PAD_MINUS:
         key[KEY_PAD_MINUS] = true;
         break;
      case ALLEGRO_KEY_E:
         key[KEY_E] = true;
         break;
      case ALLEGRO_KEY_R:
         key[KEY_R] = true;
         break;
      case ALLEGRO_KEY_Q:
         key[KEY_Q] = true;
         break;
      case ALLEGRO_KEY_F:
         key[KEY_F] = true;
         break;
      case ALLEGRO_KEY_G:
         key[KEY_G] = true;
         break;
   }
}

/************************************************
 * Checks "Key Up" events                       *
 ************************************************/
void check_key_up(ALLEGRO_EVENT *ev)
{
   switch(ev->keyboard.keycode)
   {
      case ALLEGRO_KEY_UP:
         key[KEY_UP] = false;
         break;
      case ALLEGRO_KEY_DOWN:
         key[KEY_DOWN] = false;
         break;
      case ALLEGRO_KEY_LEFT:
         key[KEY_LEFT] = false;
         break;
      case ALLEGRO_KEY_RIGHT:
         key[KEY_RIGHT] = false;
         break;
      case ALLEGRO_KEY_W:
         key[KEY_UP] = false;
         break;
      case ALLEGRO_KEY_S:
         key[KEY_DOWN] = false;
         break;
      case ALLEGRO_KEY_A:
         key[KEY_LEFT] = false;
         break;
      case ALLEGRO_KEY_D:
         key[KEY_RIGHT] = false;
         break;
      case ALLEGRO_KEY_Z:
         key[KEY_Z] = false;
         break;
      case ALLEGRO_KEY_N:
         key[KEY_N] = false;
         break;
      case ALLEGRO_KEY_LSHIFT:
         key[KEY_LSHIFT] = false;
         cond.show_mini_map = false;
         break;
      case ALLEGRO_KEY_B:
         if (cond.name_map == false)
         {
            al_save_bitmap("map.png", mini_map);
            jlog("Image of map saved to map.png");
         }
         break;
      case ALLEGRO_KEY_PAD_PLUS:
         key[KEY_PAD_PLUS] = false;
         break;
      case ALLEGRO_KEY_PAD_MINUS:
         key[KEY_PAD_MINUS] = false;
         break;
      case ALLEGRO_KEY_F2:
         if (save_map(map, display)) cond.map_saved = true;
         break;
      case ALLEGRO_KEY_F5:
         cond.map_load = true;
         break;
      case ALLEGRO_KEY_F12: //1NameMap
         cond.name_map = true;
         break;
      case ALLEGRO_KEY_F:
         key[KEY_F] = false;
         break;
      case ALLEGRO_KEY_G:
         key[KEY_G] = false;
         break;
      case ALLEGRO_KEY_P:
         cam.x = player.x - VIEWPORT_WIDTH / 2 + 16;
         cam.y = player.y - VIEWPORT_HEIGHT / 2 + 16;
         break;
      case ALLEGRO_KEY_2:
         if (item_selected < 17) item_selected++;
         if (item_selected == 17) item_selected = 1;
         break;
      case ALLEGRO_KEY_1:
         if (item_selected > 0) item_selected--;
         if (item_selected == 0) item_selected = 17;
         break;
      case ALLEGRO_KEY_E:
         key[KEY_E] = false;
         break;
      case ALLEGRO_KEY_R:
         key[KEY_R] = false;
         break;
      case ALLEGRO_KEY_V:   //Increase selected thing
         thing_selected++;
         break;
      case ALLEGRO_KEY_C:   //Decrease selected thing
         thing_selected--;
         break;
      case ALLEGRO_KEY_Q:
         key[KEY_Q] = false;
         al_set_system_mouse_cursor(display, ALLEGRO_SYSTEM_MOUSE_CURSOR_PRECISION);
         break;
      case ALLEGRO_KEY_ESCAPE:
         //program_done = true;
         break;
   }
}

/************************************************
 * Checks logic that needs to be timed by FPS   *
 ************************************************/
void check_timer_logic(ALLEGRO_EVENT *ev)
{
   static int scroll_speed;

   if (ev->timer.source == ANIM_TIMER)
   {
      if (player.cur_frame < 5)
      {
         player.cur_frame++;
      }
      if (player.cur_frame == 5)
      {
         player.cur_frame = 1;
      }
   }

   if (ev->timer.source == FPS_TIMER)
   {
      //Cam Speed is faster on zoomed out mini-map
      if (key[KEY_LSHIFT])
      {
         scroll_speed = 32;
      }
      else if (!key[KEY_LSHIFT])
      {
         scroll_speed = 4;
      }

      //Scroll controls
      if (key[KEY_UP] && cam.y > 0)
      {
         cam.y -= scroll_speed;
      }
      if (key[KEY_DOWN] && cam.y < (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT)
      {
         cam.y += scroll_speed;
      }
      if (key[KEY_LEFT] && cam.x > 0)
      {
         cam.x -= scroll_speed;
      }
      if (key[KEY_RIGHT] && cam.x < (MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH)
      {
         cam.x += scroll_speed;
      }

      if (key[KEY_Z] && key[KEY_N])
      {
         key[KEY_Z] = false;
         key[KEY_N] = false;
         int button = al_show_native_message_box(display,
                                                 "Create New Map",
                                                 "Are you sure?",
                                                 "This will clear your current map data, and you will "
                                                 "lose the current map if it's not saved.\n\n"
                                                 "Do you want to continue?",
                                                 NULL, ALLEGRO_MESSAGEBOX_YES_NO);
         if (button == 1)
         {
            destroy_map(map);
            map = NULL;
            map = create_empty_map();
            player.x = map->player_start_x;
            player.y = map->player_start_y;
            cam.x = 0;
            cam.y = 0;
            cond.map_saved = false;
         }
      }

      //Load a map, but destroy the one in memory first
      if (cond.map_load == true)
      {
         cond.map_load = false;
         if(cond.map_saved == false)
         {
            al_show_native_message_box(display,
                                      "Open Map - WARNING",
                                      "Map changes not saved!!",
                                      "If you open a map, it will clear your current changes. Consider saving first!",
                                     NULL, ALLEGRO_MESSAGEBOX_WARN);
         }
         open_file_dialog();
      }

      check_cam_bounds();

     
   }
}

/************************************************
 * When naming a map by pressing F12            *
 ************************************************/
void check_map_naming(ALLEGRO_EVENT *ev)
{
   static int caret_pos;
   caret_pos = (int)strlen(map->name);
   switch (ev->keyboard.keycode)
   {
      case ALLEGRO_KEY_ENTER:
         jlog("Map named to %s", map->name);
         cond.name_map = false;
         cond.map_saved = false;
         break;

      case ALLEGRO_KEY_BACKSPACE:
         if (caret_pos > 0) caret_pos--;
         map->name[caret_pos] = '\0';
         break;
      default:
         if (ev->keyboard.unichar >= 32 && ev->keyboard.unichar < 127)
         {
            if (caret_pos < 31)
            {
               map->name[caret_pos] = (char)toupper((int)ev->keyboard.unichar);
               caret_pos++;
               map->name[caret_pos] = '\0';
               break;
            }
         }
         break;
   }
}

/************************************************
 * Clean-ups for end of program                 *
 ************************************************/
void clean_up()
{
   destroy_map(map);

   if(event_queue) {
      al_destroy_event_queue(event_queue);
      event_queue = NULL;
      jlog("Event queue destroyed.");
   }

   if(FPS_TIMER) {
      al_destroy_timer(FPS_TIMER);
      FPS_TIMER = NULL;
      jlog("FPS_TIMER destroyed.");
   }

   if(ANIM_TIMER) {
      al_destroy_timer(ANIM_TIMER);
      ANIM_TIMER = NULL;
      jlog("Animation timer destroyed.");
   }

   al_destroy_display(display);
   jlog("Display Destroyed.");

   jlog("***QUITTING***\n\n");
}

/************************************************
 *                                              *
 *                 HERE IS                      *
 *                   MAIN                       *
 *                                              *
 ************************************************/
int main(int argc, char **argv)
{
   bool redraw = true;

   if (init_game() != 0)
   {
      jlog("Failed to init game!\n");
      return -1;
   }

   map = create_empty_map(); //Create an empty map
   if (map == NULL)
   {
      jlog("In file %s, Line %d. Couldn't create an empty map!" __FILE__, __LINE__);
      return -1;
   }
   jlog("Empty map created.");

   al_set_target_bitmap(game);
   al_draw_bitmap(stat_border, 0, 0, 0);

   //This is the main loop for now
   bool program_done = false;
   al_start_timer(FPS_TIMER);
   jlog("FPS timer started.");
   al_start_timer(ANIM_TIMER);
   jlog("Animation timer started.");
   while(!program_done)
   {

      ALLEGRO_EVENT ev;
      al_wait_for_event(event_queue, &ev);

      if (ev.type == ALLEGRO_EVENT_TIMER)
      {
         if (!cond.name_map)
         {
            check_timer_logic(&ev);
         }     

         redraw = true;
      }
      else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
      {
         if (!cond.map_saved)
         {
            int button = al_show_native_message_box(display, "WARNING!",
                                                    "Map changes not saved!",
                                                    "Are you sure you want to quit?",
                                                    NULL, ALLEGRO_MESSAGEBOX_YES_NO);
            if (button == 1)
            {
               program_done = true;
            }
         }
         else
         {
            program_done = true;
         }
      }
      else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
      {
         if (!cond.name_map) check_key_down(&ev);
      }
      else if (ev.type == ALLEGRO_EVENT_KEY_UP)
      {
         if (!cond.name_map) check_key_up(&ev);
      }
      else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)
      {
         sp_mouse.x = ev.mouse.x;
         sp_mouse.y = ev.mouse.y;
      }

      check_click_in_viewport();
      check_tile_selection();

      if (ev.type == ALLEGRO_EVENT_KEY_CHAR )
      {
        if (cond.name_map == true)
        {
           check_map_naming(&ev);
        }
      }

      if (redraw && al_is_event_queue_empty(event_queue))
      {
         redraw = false;
         update_screen();
      }
   }

   clean_up();
   return 0;
}
