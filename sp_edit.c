#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>

#include "memwatch/memwatch.h"

#include "sp_main.h"
#include "sp_map.h"
#include "sp_edit.h"

const float FPS = 60;

bool redraw = true;

const int DISPLAY_WIDTH = 320 * DISPLAY_MULTIPLIER;
const int DISPLAY_HEIGHT = 200 * DISPLAY_MULTIPLIER;

t_cam cam;
t_mouse sp_mouse;
t_map *map = NULL;

t_conditional cond = {false, false, false};

const char *filename;

enum KEYS {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LSHIFT, KEY_PAD_PLUS, KEY_PAD_MINUS};
bool key[7] = {false, false, false, false, false, false, false};

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *FPS_TIMER = NULL;
ALLEGRO_MOUSE_STATE mouse;

ALLEGRO_FONT *reg_font = NULL;

//Bitmaps that get loaded from disk
ALLEGRO_BITMAP *tile_sheet = NULL;
ALLEGRO_BITMAP *bg = NULL;
ALLEGRO_BITMAP *stat_border = NULL;
ALLEGRO_BITMAP *player_start = NULL;

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
   jlog("Allegro initialized.");

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
   display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
   if(!display)
   {
      jlog("Failed to create display!");
      return -1;
   }
   jlog("Display Created.");


   stat_border = al_load_bitmap("data/status_border.png");
   bg = al_load_bitmap("data/bg_1.png");
   player_start = al_load_bitmap("data/player_start.png");

   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
   mini_map = al_create_bitmap(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE);

   //Create the game bitmap that needs to be stretched to display
   game = al_create_bitmap(320, 200);

   tile_sheet = al_load_bitmap("data/tile_sheet.png");
   if (tile_sheet == NULL) { jlog("Couldn't load tile_sheet.png"); }
   jlog("tile_sheet.png loaded.");

   reg_font = al_load_ttf_font(FONT_FILE, 4 * DISPLAY_MULTIPLIER, 0);
   if(!reg_font)
   {
      jlog("Failed to load %s", FONT_FILE);
      return -1;
   }
   jlog("%s loaded.", FONT_FILE);

   al_register_event_source(event_queue, al_get_display_event_source(display));
   al_register_event_source(event_queue, al_get_timer_event_source(FPS_TIMER));
   al_register_event_source(event_queue, al_get_keyboard_event_source());
   al_register_event_source(event_queue, al_get_mouse_event_source());

   al_clear_to_color(al_map_rgb(0, 0, 0));

   al_start_timer(FPS_TIMER);
   jlog("Timer started.");

   //load tile sheet into an array of bitmaps
   al_draw_text(reg_font, al_map_rgb(255,255,255), 16, 16 , 0, "Loading...");
   al_flip_display();

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
   al_draw_bitmap(player_start, map->player_start_x, map->player_start_y, 0);
}

/************************************************
 * Draws all the texts, info, and helpers       *
 ************************************************/
void show_info_stuff()
{
   if (cond.show_mini_map == false)
   {
      al_draw_bitmap(tile_sheet, 226 * DISPLAY_MULTIPLIER, 16 * DISPLAY_MULTIPLIER, 0);
      al_draw_rectangle(convert_index_to_pixel_xy(sp_mouse.tile_selection, 16, TILE_SIZE, RETURN_X) + (226 * DISPLAY_MULTIPLIER),
                        convert_index_to_pixel_xy(sp_mouse.tile_selection, 16, TILE_SIZE, RETURN_Y) + (16 * DISPLAY_MULTIPLIER),
                        convert_index_to_pixel_xy(sp_mouse.tile_selection, 16, TILE_SIZE, RETURN_X) + (226 * DISPLAY_MULTIPLIER) + TILE_SIZE,
                        convert_index_to_pixel_xy(sp_mouse.tile_selection, 16, TILE_SIZE, RETURN_Y) + (16 * DISPLAY_MULTIPLIER) + TILE_SIZE,
                        al_map_rgb(255,255,0),
                        2);
      al_draw_textf(
                    reg_font,
                    al_map_rgb(255,255,255),
                    16 * DISPLAY_MULTIPLIER,
                    6 * DISPLAY_MULTIPLIER,
                    ALLEGRO_ALIGN_LEFT, "map->position %d, %d",
                    sp_mouse.over_tile_x,
                    sp_mouse.over_tile_y
                    );

      if (cond.name_map)
      {
         al_draw_filled_circle(16 * DISPLAY_MULTIPLIER - 2 * DISPLAY_MULTIPLIER, 4 * DISPLAY_MULTIPLIER,
                               2 * DISPLAY_MULTIPLIER,
                               al_map_rgb(255, 0, 0));
      }
      al_draw_textf(reg_font,
                    al_map_rgb(255,255,255),
                    16 * DISPLAY_MULTIPLIER,
                    2 * DISPLAY_MULTIPLIER,
                    ALLEGRO_ALIGN_LEFT,
                    "%s",
                    map->name);
      al_draw_textf(reg_font,
                    al_map_rgb(255,255,255),
                    226 * DISPLAY_MULTIPLIER,
                    10 * DISPLAY_MULTIPLIER,
                    0,
                    "Tile Selected: %d",
                    sp_mouse.tile_selection);

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
      al_draw_bitmap(player_start, map->player_start_x - cam.x, map->player_start_y - cam.y, 0);
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
         draw_map(view_port, tile_sheet, bg, &cam, map);

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
                               256,
                               16,
                               0);
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

   if (cond.show_mini_map == false) al_draw_scaled_bitmap(game, 0, 0, 320, 200, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);

   if (cond.show_mini_map)
   {
      al_draw_scaled_bitmap(mini_map, 0, 0, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT , 0);
   }

   show_info_stuff();
   al_flip_display();
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
         }
         else if (mouse.buttons & 2)
         {
            map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].empty_tile = true;
         }
         else if (mouse.buttons & 4)
         {
            map->player_start_x = sp_mouse.over_tile_x * TILE_SIZE;
            map->player_start_y = sp_mouse.over_tile_y * TILE_SIZE;
         }
      }
      else
      {
         sp_mouse.over_tile_x = 0;
         sp_mouse.over_tile_y = 0;
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
   ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
   al_append_path_component(path, "data/maps");
   al_set_path_filename(path, (const char *)"map.spl");
   filename = al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP);

   ALLEGRO_FILECHOOSER *file_dialog = al_create_native_file_dialog(filename,
                                                                   "Open Map",
                                                                   "*.spl",
                                                                   ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
   if(!file_dialog) return false;

   if(!al_show_native_file_dialog(display, file_dialog)) {
      if(file_dialog) al_destroy_native_file_dialog(file_dialog);
      jlog("Open File dialog closed.");
      return false;
   }

   filename = al_get_native_file_dialog_path(file_dialog, 0);
   jlog("%s", filename);

   al_destroy_native_file_dialog(file_dialog);

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
         save_map(map, display);
         break;
      case ALLEGRO_KEY_F3:
         cond.map_load = true;
         break;
      case ALLEGRO_KEY_F12:
         cond.name_map = true;
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

      check_click_in_viewport();
      check_tile_selection();

      //Load a map, but destroy the one in memory first
      if (cond.map_load == true)
      {
         cond.map_load = false;
         if (open_file_dialog())
         {
            destroy_map(map);
            map = NULL;
            map = load_map(filename);
            if (map == NULL)
            {
               jlog("Something went wrong with map loading!");
               destroy_map(map);
               map = NULL;
            }
         }
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
   caret_pos = strlen(map->name);

   switch (ev->keyboard.keycode)
   {
      case ALLEGRO_KEY_ENTER:
         jlog("Map named to %s", map->name);
         cond.name_map = false;
         break;

      case ALLEGRO_KEY_BACKSPACE:
         if (caret_pos > 0)
         {
            map->name[caret_pos-1] = '\0';
            --caret_pos;
            break;
         }
         break;
      default:
         if (ev->keyboard.unichar >= 32 && ev->keyboard.unichar < 127)
         {
            if (caret_pos < 31)
            {
               map->name[caret_pos] = ev->keyboard.unichar;
               map->name[caret_pos+1] = '\0';
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
         break;
      }
      else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
      {
         check_key_down(&ev);
      }
      else if (ev.type == ALLEGRO_EVENT_KEY_UP)
      {
         check_key_up(&ev);
      }
      else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)
      {
         sp_mouse.x = ev.mouse.x;
         sp_mouse.y = ev.mouse.y;
      }


      if (ev.type == ALLEGRO_EVENT_KEY_CHAR && al_is_event_queue_empty(event_queue))
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
