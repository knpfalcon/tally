#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include "sp_main.h"
#include "sp_map.h"

const float FPS = 60;

bool redraw = true;

const int DISPLAY_WIDTH = 320 * DISPLAY_MULTIPLIER;
const int DISPLAY_HEIGHT = 200 * DISPLAY_MULTIPLIER;

const int VIEWPORT_WIDTH = 208;
const int VIEWPORT_HEIGHT = 160;
const int VIEWPORT_X = 16;
const int VIEWPORT_Y = 16;

float CAM_X = 0;
float CAM_Y = 0;

int mouse_x = 0;
int mouse_y = 0;

int mouse_over_tile_x = 0;
int mouse_over_tile_y = 0;

bool show_mini_map = false;

enum KEYS {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LSHIFT};
bool key[5] = {false, false, false, false, false };

t_map *map = NULL;

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *FPS_TIMER = NULL;

ALLEGRO_FONT *reg_font = NULL;
ALLEGRO_BITMAP *view_port = NULL;
ALLEGRO_BITMAP *stat_border = NULL;
ALLEGRO_BITMAP *game = NULL;
ALLEGRO_BITMAP *tile_sheet = NULL;
ALLEGRO_BITMAP *mini_map = NULL;

ALLEGRO_MOUSE_STATE mouse;

/////////////////////////////////////////////////
//Initiate everything the game needs at startup
///////////////////////////////////////////////
int init_game()
{
   //Init Allegro
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
   jlog("Image addon initialized.");

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


   //Font Addons
   if(!al_init_font_addon())
   {
      jlog("Failed to install fonts addon!");
      return -1;
   }
   jlog("Fonts addon installed.");

   if(!al_init_ttf_addon())
   {
      jlog("Failed to install ttf addon!");
      return -1;
   }
   jlog("TTF addon installed.");

   //Primitives
   if (!al_init_primitives_addon())
   {
      jlog("Failed to install primitives addon!");
      return -1;
   }
   jlog("Primitives addon initialized.");

   //Create Display
   display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
   if(!display)
   {
      jlog("Failed to create display!");
      return -1;
   }
   jlog("Display Created.");

   //Create viewport bitmap
   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
   stat_border = al_load_bitmap("status_border.png");

   //Create the game bitmap that needs to be stretched to display
   game = al_create_bitmap(320, 200);
   mini_map = al_create_bitmap(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE);

   tile_sheet = al_load_bitmap("tile_sheet.png");
   if (tile_sheet == NULL) { jlog("Couldn't load tile_sheet.png"); }
   jlog("tile_sheet.png loaded.");


   reg_font = al_load_ttf_font(FONT_FILE, 8, 0);
   if(!reg_font)
   {
      jlog("Failed to load %s", FONT_FILE);
      return -1;
   }
   jlog("%s loaded.", FONT_FILE);

   //Fill with random 1's and 0's
   srand(time(NULL));
//   for (int i = 0; i < MAP_WIDTH + MAP_HEIGHT * MAP_WIDTH; i++)
//   {
//      map->position[i].tile = (unsigned char)rand() % 4;
//   }
//   jlog("Map randomly filled.");
//   for (int t = 0; t < MAP_HEIGHT * MAP_WIDTH; t++)
//   {
//            printf("%d", map[t]);
//   }

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


void draw_tiles()
{
   al_set_target_bitmap(view_port);
   al_clear_to_color(al_map_rgba(0, 0, 0, 0));
                                                            //Which tiles are in view? The view port size divided by the tile size plus 1.
   int y_tiles_in_view = VIEWPORT_HEIGHT / TILE_SIZE + 1;   //Should be 10 tiles +1 row off view
   int x_tiles_in_view = VIEWPORT_WIDTH / TILE_SIZE + 1;    //Should be 13 tiles +1 Column off view


   int sx = CAM_X / TILE_SIZE;      //This is the tile to draw from according to camera position
   int sy = CAM_Y / TILE_SIZE;      //This is the tile to draw from according to camera position

   for (int y = sy; y < sy + y_tiles_in_view; y++)
   {
      for (int x = sx; x < sx + x_tiles_in_view; x++)
      {
         if ((x < MAP_WIDTH) && (x > -1) && (y < MAP_HEIGHT) && (y > -1))
         {
            if ((map->position[x + y * MAP_WIDTH].tile == 1))
            {
               al_draw_bitmap_region(tile_sheet, TILE001, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE) - CAM_X, (y * TILE_SIZE) - CAM_Y, 0); //Draw the tile, subtracting the camera position
            }
            if ((map->position[x + y * MAP_WIDTH].tile == 2))
            {
               al_draw_bitmap_region(tile_sheet, TILE002, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE) - CAM_X, (y * TILE_SIZE) - CAM_Y, 0); //Draw the tile, subtracting the camera position
            }
            if ((map->position[x + y * MAP_WIDTH].tile == 3))
            {
               al_draw_bitmap_region(tile_sheet, TILE003, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE) - CAM_X, (y * TILE_SIZE) - CAM_Y, 0); //Draw the tile, subtracting the camera position
            }
         }
      }
   }

}

void draw_mini_map()
{
   al_set_target_bitmap(mini_map);
   al_clear_to_color(al_map_rgb(0,0,0));
   //al_lock_bitmap(mini_map, al_get_bitmap_format(mini_map), ALLEGRO_LOCK_READWRITE);
   for (int y = 0; y < MAP_HEIGHT; y++)
   {
      for (int x = 0; x < MAP_WIDTH; x++)
      {
         if ((map->position[x + y * MAP_WIDTH].tile == 1))
         {
            al_draw_bitmap_region(tile_sheet, TILE001, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE), (y * TILE_SIZE), 0);
         }
         if ((map->position[x + y * MAP_WIDTH].tile == 2))
         {
            al_draw_bitmap_region(tile_sheet, TILE002, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE), (y * TILE_SIZE), 0);
         }
         if ((map->position[x + y * MAP_WIDTH].tile == 3))
         {
            al_draw_bitmap_region(tile_sheet, TILE003, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE), (y * TILE_SIZE), 0);
         }
      }
   }
   //al_unlock_bitmap(mini_map);
}

void update_screen()
{

   draw_mini_map();
   if (show_mini_map == false)
   {
      draw_tiles();
      al_set_target_bitmap(game);
      al_clear_to_color(al_map_rgb(0,0,0));
      al_draw_bitmap(stat_border, 0, 0, 0);
      al_draw_bitmap(view_port, 16, 16, 0);
      al_draw_textf(reg_font, al_map_rgb(255,255,255), 234, 17, ALLEGRO_ALIGN_LEFT, "map->position");
      al_draw_textf(reg_font, al_map_rgb(255,255,255), 234, 27, ALLEGRO_ALIGN_LEFT, "%d, %d", mouse_over_tile_x, mouse_over_tile_y);
   }

   al_set_target_bitmap(mini_map);
   if (show_mini_map) al_draw_rectangle(CAM_X, CAM_Y, CAM_X + VIEWPORT_WIDTH, CAM_Y + VIEWPORT_HEIGHT, al_map_rgb(255,255,255),1);

   al_set_target_backbuffer(display);

   if (show_mini_map == false) al_draw_scaled_bitmap(game, 0, 0, 320, 200, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);

   if (show_mini_map)
   {
      al_draw_scaled_bitmap(mini_map, 0, 0, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT , 0);
   }

   al_flip_display();
}

//////////////////////////////////////////////////////
//Clean up everything for now (Replacing this later)
////////////////////////////////////////////////////
void clean_up()
{
   al_destroy_display(display);
   al_destroy_bitmap(view_port);
   al_destroy_bitmap(stat_border);
   al_destroy_bitmap(game);
   al_destroy_bitmap(tile_sheet);
   al_destroy_bitmap(mini_map);
   al_destroy_event_queue(event_queue);
   al_destroy_timer(FPS_TIMER);
   destroy_map(map);
   jlog("***CLEANING UP AND QUITTING***\n\n");
}

int main(int argc, char **argv)
{

   int scroll_speed = 16;

   if (init_game() != 0)
   {
      jlog("Failed to init game!\n");
      clean_up();
      return 100;
   }

   map = create_empty_map(); //Create an empty map
   if (map == NULL)
   {
      jlog("In file %s, Line %d. Couldn't create an empty map!" __FILE__, __LINE__);
      return -505;
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
            if (key[KEY_UP] && CAM_Y > 0)
            {
               CAM_Y -= scroll_speed;
            }
            if (key[KEY_DOWN] && CAM_Y < (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT)
            {
               CAM_Y += scroll_speed;
            }
            if (key[KEY_LEFT] && CAM_X > 0)
            {
               CAM_X -= scroll_speed;
            }
            if (key[KEY_RIGHT] && CAM_X < (MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH)
            {
               CAM_X += scroll_speed;
            }

            // If mouse clicks in view port. I'm putting this here for now.
            al_get_mouse_state(&mouse);
            if (show_mini_map == false)
            {
               //If mouse is inside view port
               if ((mouse_x > VIEWPORT_X * DISPLAY_MULTIPLIER) && (mouse_x < (VIEWPORT_WIDTH + VIEWPORT_X) * DISPLAY_MULTIPLIER) && (mouse_y > VIEWPORT_Y * DISPLAY_MULTIPLIER) && (mouse_y < (VIEWPORT_HEIGHT + VIEWPORT_Y) * DISPLAY_MULTIPLIER))
               {
                  mouse_over_tile_x = ( (((mouse_x - (16 * DISPLAY_MULTIPLIER)) + (CAM_X * DISPLAY_MULTIPLIER)) / TILE_SIZE) / DISPLAY_MULTIPLIER );
                  mouse_over_tile_y = ( (((mouse_y - (16 * DISPLAY_MULTIPLIER)) + (CAM_Y * DISPLAY_MULTIPLIER)) / TILE_SIZE) / DISPLAY_MULTIPLIER );

                  if (mouse.buttons & 1)
                  {
                  map->position[mouse_over_tile_x + mouse_over_tile_y * MAP_WIDTH].tile = 1; //(unsigned char)rand() % 4;
                  }
                  else if (mouse.buttons & 2)
                  {
                  map->position[mouse_over_tile_x + mouse_over_tile_y * MAP_WIDTH].tile = 0;
                  }
               }
               else
               {
                  mouse_over_tile_x = 0;
                  mouse_over_tile_y = 0;
               }
            }

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
            case ALLEGRO_KEY_ESCAPE:
               program_done = true;
               break;
         }
      }
      else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)
      {
         mouse_x = ev.mouse.x;
         mouse_y = ev.mouse.y;
      }

      if (redraw && al_is_event_queue_empty(event_queue))
      {
         //Check to make sure camera is not out of bounds.
            if (CAM_X < 0) CAM_X = 0;
            if (CAM_X > (MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH) CAM_X = (MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH;
            if (CAM_Y < 0) CAM_Y = 0;
            if (CAM_Y > (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT) CAM_Y = (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT;


         redraw = false;
         update_screen();
      }



   }
   clean_up();
   return 0;
}
