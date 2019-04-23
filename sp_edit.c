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


const float FPS = 60;

bool redraw = true;

const int DISPLAY_WIDTH = 320 * DISPLAY_MULTIPLIER;
const int DISPLAY_HEIGHT = 200 * DISPLAY_MULTIPLIER;

t_cam cam;
t_mouse sp_mouse;
t_map *map = NULL;

bool show_mini_map = false;

const char *filename;

enum KEYS {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LSHIFT, KEY_PAD_PLUS, KEY_PAD_MINUS};
bool key[7] = {false, false, false, false, false, false, false };

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *FPS_TIMER = NULL;

ALLEGRO_FONT *reg_font = NULL;
ALLEGRO_BITMAP *view_port = NULL;
ALLEGRO_BITMAP *stat_border = NULL;
ALLEGRO_BITMAP *game = NULL;
ALLEGRO_BITMAP *tile_sheet = NULL;
ALLEGRO_BITMAP *mini_map = NULL;
ALLEGRO_BITMAP *bg = NULL;

ALLEGRO_MOUSE_STATE mouse;

/////////////////////////////////////////////////
//Initiate everything the game needs at startup
///////////////////////////////////////////////
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
      jlog("Faile to initialize native dialog addon!");
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

   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
   stat_border = al_load_bitmap("data/status_border.png");
   bg = al_load_bitmap("data/bg_1.png");
   mini_map = al_create_bitmap(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE);

   //Create the game bitmap that needs to be stretched to display
   game = al_create_bitmap(320, 200);

   tile_sheet = al_load_bitmap("data/tile_sheet.png");
   if (tile_sheet == NULL) { jlog("Couldn't load tile_sheet.png"); }
   jlog("tile_sheet.png loaded.");


   reg_font = al_load_ttf_font(FONT_FILE, 16, 0);
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

   jlog("Game initialized.");
   return 0;
}

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
            al_draw_bitmap_region(tile_sheet, map->position[x + y * MAP_WIDTH].tile_x * TILE_SIZE, map->position[x + y * MAP_WIDTH].tile_y * TILE_SIZE, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE), (y * TILE_SIZE), 0);
         }

      }
   }
}

void show_info_stuff()
{
   if (show_mini_map == false)
   {
      al_draw_bitmap(tile_sheet, 226 * DISPLAY_MULTIPLIER, 16 * DISPLAY_MULTIPLIER, 0);
      al_draw_rectangle(sp_mouse.tile_selection_x * TILE_SIZE +(226 * DISPLAY_MULTIPLIER), sp_mouse.tile_selection_y * TILE_SIZE + (16 * DISPLAY_MULTIPLIER),
                        sp_mouse.tile_selection_x * TILE_SIZE +(226 * DISPLAY_MULTIPLIER) + TILE_SIZE, sp_mouse.tile_selection_y  * TILE_SIZE+ (16 * DISPLAY_MULTIPLIER) + TILE_SIZE,
                        al_map_rgb(255,255,0), 2);
      al_draw_textf(reg_font, al_map_rgb(255,255,255), 16 * DISPLAY_MULTIPLIER, 4 * DISPLAY_MULTIPLIER, ALLEGRO_ALIGN_LEFT, "map->position %d, %d", sp_mouse.over_tile_x, sp_mouse.over_tile_y);
      al_draw_textf(reg_font, al_map_rgb(255,255,255), 16 * DISPLAY_MULTIPLIER, 2 * DISPLAY_MULTIPLIER, ALLEGRO_ALIGN_LEFT, "%s", map->name);
      al_draw_textf(reg_font, al_map_rgb(255,255,255), 226 * DISPLAY_MULTIPLIER , 13 * DISPLAY_MULTIPLIER, 0, "x:%d, y:%d", sp_mouse.tile_selection_x, sp_mouse.tile_selection_y);
      al_draw_textf(reg_font, al_map_rgb(255,255,255), 256 * DISPLAY_MULTIPLIER, 13 * DISPLAY_MULTIPLIER, 0, "Tile Selected: %d", sp_mouse.tile_selection);

   }
}

void update_screen()
{
   draw_mini_map();
   if (show_mini_map == false)
   {
      if (map != NULL) draw_map(view_port, tile_sheet, bg, &cam, map);
      al_set_target_bitmap(game);
      al_clear_to_color(al_map_rgb(0,0,0));
      al_draw_bitmap(stat_border, 0, 0, 0);
      al_draw_bitmap(view_port, 16, 16, 0);
      al_draw_bitmap_region(tile_sheet, sp_mouse.tile_selection_x * TILE_SIZE, sp_mouse.tile_selection_y * TILE_SIZE, TILE_SIZE, TILE_SIZE, 256, 16, 0);
   }

   al_set_target_bitmap(mini_map);
   if (show_mini_map) al_draw_rectangle(cam.x, cam.y, cam.x + VIEWPORT_WIDTH, cam.y + VIEWPORT_HEIGHT, al_map_rgb(255,255,255),1);

   al_set_target_backbuffer(display);


   if (show_mini_map == false) al_draw_scaled_bitmap(game, 0, 0, 320, 200, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);

   if (show_mini_map)
   {
      al_draw_scaled_bitmap(mini_map, 0, 0, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT , 0);
   }


   show_info_stuff();
   al_flip_display();
}

void check_click_in_viewport() // If mouse clicks in view port. I'm putting this here for now.
{
   al_get_mouse_state(&mouse);
   if (show_mini_map == false)
   {
      //If mouse is inside view port
      if ((sp_mouse.x > VIEWPORT_X * DISPLAY_MULTIPLIER) && (sp_mouse.x < (VIEWPORT_WIDTH + VIEWPORT_X) * DISPLAY_MULTIPLIER) && (sp_mouse.y > VIEWPORT_Y * DISPLAY_MULTIPLIER) && (sp_mouse.y < (VIEWPORT_HEIGHT + VIEWPORT_Y) * DISPLAY_MULTIPLIER))
      {
         sp_mouse.over_tile_x = ( (((sp_mouse.x - (16 * DISPLAY_MULTIPLIER)) + (cam.x * DISPLAY_MULTIPLIER)) / TILE_SIZE) / DISPLAY_MULTIPLIER );
         sp_mouse.over_tile_y = ( (((sp_mouse.y - (16 * DISPLAY_MULTIPLIER)) + (cam.y * DISPLAY_MULTIPLIER)) / TILE_SIZE) / DISPLAY_MULTIPLIER );

         if (mouse.buttons & 1 && map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].empty_tile == true)
         {
            map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].empty_tile = false;
            map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].tile_x = sp_mouse.tile_selection_x;
            map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].tile_y = sp_mouse.tile_selection_y;
         }
         else if (mouse.buttons & 2)
         {
            map->position[sp_mouse.over_tile_x + sp_mouse.over_tile_y * MAP_WIDTH].empty_tile = true;
         }
      }
      else
      {
         sp_mouse.over_tile_x = 0;
         sp_mouse.over_tile_y = 0;
      }
   }
}

void check_cam_bounds() //Check to make sure camera is not out of bounds.
{
   if (cam.x < 0) cam.x = 0;
   if (cam.x > (MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH) cam.x = (MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH;
   if (cam.y < 0) cam.y = 0;
   if (cam.y > (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT) cam.y = (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT;
}

void check_tile_selection()
{
   if (mouse.z < sp_mouse.z || key[KEY_PAD_MINUS])
   {
      sp_mouse.z = mouse.z;

      if (sp_mouse.tile_selection_x > 0)
      {
         sp_mouse.tile_selection_x--;
      }
      else if (sp_mouse.tile_selection_x == 0 && sp_mouse.tile_selection_y > 0)
      {
         sp_mouse.tile_selection_x = 15;
         sp_mouse.tile_selection_y--;
      }
      else if (sp_mouse.tile_selection_x == 0 && sp_mouse.tile_selection_y == 0)
      {
         sp_mouse.tile_selection_x = 15;
         sp_mouse.tile_selection_y = 15;
      }
      sp_mouse.tile_selection = sp_mouse.tile_selection_x + sp_mouse.tile_selection_y * 16;

   }
   if (mouse.z > sp_mouse.z || key[KEY_PAD_PLUS])
   {
      sp_mouse.z = mouse.z;

      if (sp_mouse.tile_selection_x < 15)
      {
         sp_mouse.tile_selection_x++;
      }
      else if (sp_mouse.tile_selection_x == 15 && sp_mouse.tile_selection_y < 15)
      {
         sp_mouse.tile_selection_x = 0;
         sp_mouse.tile_selection_y++;
      }
      else if (sp_mouse.tile_selection_x == 15 && sp_mouse.tile_selection_y == 15)
      {
         sp_mouse.tile_selection_x = 0;
         sp_mouse.tile_selection_y = 0;
      }

      sp_mouse.tile_selection = sp_mouse.tile_selection_x + sp_mouse.tile_selection_y * 16;

   }
}

void clean_up()
{


   destroy_map(map);                      jlog("Destroying map.");
//   al_destroy_bitmap(view_port);          jlog("Destroying view_port.");
//   al_destroy_bitmap(stat_border);        jlog("Destroying stat_border.");
//   al_destroy_bitmap(game);               jlog("Destroying game.");
//   al_destroy_bitmap(tile_sheet);         jlog("Destroying tile_sheet.");
//   al_destroy_bitmap(mini_map);           jlog("Destroying mini_map.");
//   al_destroy_bitmap(bg);                 jlog("Destroying bg.");
//   al_destroy_event_queue(event_queue);   jlog("Destroying event_queue.");
//   al_destroy_timer(FPS_TIMER);           jlog("Destroying FPS_TIMER.");
//   al_shutdown_native_dialog_addon();     jlog("Destroying al_shutdown_native_dialog_addon.");
   al_destroy_display(display);           jlog("Destroying display.");

   jlog("***CLEANING UP AND QUITTING***\n\n");
}

bool open_file_dialog()
{
   ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
   al_append_path_component(path, "data/maps");
   al_set_path_filename(path, (const char *)"map.spl");
   filename = al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP);

   ALLEGRO_FILECHOOSER *file_dialog = al_create_native_file_dialog(filename, "Open Map", "*.spl", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
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

int main(int argc, char **argv)
{
   bool load = false;
   int scroll_speed = 16;


   if (init_game() != 0)
   {
      jlog("Failed to init game!\n");
      goto CLEAN_UP;
   }

   map = create_empty_map(); //Create an empty map
   if (map == NULL)
   {
      jlog("In file %s, Line %d. Couldn't create an empty map!" __FILE__, __LINE__);
      goto CLEAN_UP;
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
         if (ev.timer.source == FPS_TIMER)
         {
            //Scroll speed
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

            if (load == true)
            {
               load = false;
               if (open_file_dialog())
               {
                  destroy_map(map);
                  map = NULL;
                  map = load_map(filename);
                  if (map == NULL)
                  {
                     jlog("Something went wrong with map loading!");
                     return -99;
                  }
               }
            }

            check_cam_bounds();
            redraw = true;
         }

      }
      else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
      {
         break;
      }
      else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
      {
         switch(ev.keyboard.keycode)
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
               show_mini_map = true;
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
      else if (ev.type == ALLEGRO_EVENT_KEY_UP)
      {
         switch(ev.keyboard.keycode)
         {
            case ALLEGRO_KEY_UP:
               key[KEY_UP] = false;
               break;
            case ALLEGRO_KEY_DOWN:
               key[KEY_DOWN] = false;
               break;if (show_mini_map)
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
               show_mini_map = false;
               break;
            case ALLEGRO_KEY_B:
               al_save_bitmap("map.png", mini_map);
               jlog("Map saved to map.png");
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
               load = true;
               break;
            case ALLEGRO_KEY_ESCAPE:
               //program_done = true;
               break;
         }
      }
      else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)
      {
         sp_mouse.x = ev.mouse.x;
         sp_mouse.y = ev.mouse.y;
      }

      if (redraw && al_is_event_queue_empty(event_queue))
      {


         redraw = false;
         update_screen();
      }
   }

CLEAN_UP:
   clean_up();
   return 0;
}
