#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>


#define ALLEGRO_USE_CONSOLE
#define TILE001 448, 112 //Macro hack for tile_sheet coordinates... It works, by God!
#define TILE002 400, 112
#define TILE003 128, 32
#define TILE_SIZE 16

#define MAP_WIDTH 128
#define MAP_HEIGHT 90

#define FPS 200

bool redraw = true;

const int DISPLAY_WIDTH = 1920;
const int DISPLAY_HEIGHT = 1200;

const int VIEWPORT_WIDTH = 208;
const int VIEWPORT_HEIGHT = 160;

int CAM_X = 0;
int CAM_Y = 0;

enum KEYS {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT};
bool key[4] = {false, false, false, false };

int map[MAP_WIDTH * MAP_HEIGHT] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                    1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
                                    1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
                                    1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
                                    1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
                                    1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1,
                                    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
                                    1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                                    1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                                    1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

ALLEGRO_BITMAP *view_port = NULL;
ALLEGRO_BITMAP *stat_border = NULL;
ALLEGRO_BITMAP *game = NULL;
ALLEGRO_BITMAP *tile_sheet = NULL;

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *timer = NULL;

///////////////////////////////////////////
//Function to print to console and a FILE
/////////////////////////////////////////
void jlog(char *format, ...)
{
   va_list v_ptr;
   FILE *fp;

   fp = fopen("log.txt", "at");
   if (fp)
   {
      va_start(v_ptr, format);

      vfprintf(fp, format, v_ptr);
      fprintf(fp, "\n");

      vprintf(format, v_ptr);
      printf("\n");

      va_end(v_ptr);

      fclose(fp);
    }

}

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

   //Create Display
   display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
   if(!display)
   {
      jlog("Failed to create display!");
      return -1;
   }
   jlog("Display Created.");

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

   timer = al_create_timer(1.0 / FPS);
   if(!timer)
   {
      jlog("Failed to create timer!");
      return -1;
   }
   jlog("Timer created.");

   if (!al_install_keyboard())
   {
      jlog("Failed to install keyboard!");
      return -1;
   }
   jlog("Keyboard installed.");

   //Create viewport bitmap
   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
   stat_border = al_load_bitmap("status_border.png");

   //Create the game bitmap that needs to be stretched to display
   game = al_create_bitmap(320, 200);

   tile_sheet = al_load_bitmap("tile_sheet.png");
   if (tile_sheet == NULL) { jlog("Couldn't load tile_sheet.png"); }
   jlog("tile_sheet.png loaded.");


   //Fill with random 1's and 0's
   srand(time(NULL));
   for (int i = 0; i < MAP_WIDTH * MAP_HEIGHT; i++)
   {
      map[i] = rand() % 4;
   }
//   for (int t = 0; t < MAP_HEIGHT * MAP_WIDTH; t++)
//   {
//            printf("%d", map[t]);
//   }

   al_register_event_source(event_queue, al_get_display_event_source(display));
   al_register_event_source(event_queue, al_get_timer_event_source(timer));
   al_register_event_source(event_queue, al_get_keyboard_event_source());

   al_clear_to_color(al_map_rgb(0, 0, 0));

   al_start_timer(timer);
   jlog("Timer started.");

   jlog("Game initialized.");
   return 0;
}


void draw_tiles()
{
   al_set_target_bitmap(view_port);

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
            if ((map[x + y * MAP_WIDTH] == 1))
            {
               al_draw_bitmap_region(tile_sheet, TILE001, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE) - CAM_X, (y * TILE_SIZE) - CAM_Y, 0); //Draw the tile, subtracting the camera position
            }
            if ((map[x + y * MAP_WIDTH] == 2))
            {
               al_draw_bitmap_region(tile_sheet, TILE002, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE) - CAM_X, (y * TILE_SIZE) - CAM_Y, 0); //Draw the tile, subtracting the camera position
            }
            if ((map[x + y * MAP_WIDTH] == 3))
            {
               al_draw_bitmap_region(tile_sheet, TILE003, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE) - CAM_X, (y * TILE_SIZE) - CAM_Y, 0); //Draw the tile, subtracting the camera position
            }
         }
      }
   }
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
   al_destroy_event_queue(event_queue);
   al_destroy_timer(timer);
   jlog("***CLEANING UP AND QUITTING***\n\n");
}

int main(int argc, char **argv)\
{

   if (init_game() != 0)
   {
      jlog("Failed to init game!\n");
      clean_up();
      return 100;
   }

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
         if (key[KEY_UP] && CAM_Y > 0)
         {
            CAM_Y--;
         }
         if (key[KEY_DOWN] && CAM_Y < MAP_HEIGHT * TILE_SIZE - VIEWPORT_HEIGHT)
         {
            CAM_Y++;
         }
         if (key[KEY_LEFT] && CAM_X > 0)
         {
            CAM_X--;
         }
         if (key[KEY_RIGHT] && CAM_X < MAP_WIDTH * TILE_SIZE - VIEWPORT_WIDTH)
         {
            CAM_X++;
         }
         redraw = true;
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
               break;
            case ALLEGRO_KEY_LEFT:
               key[KEY_LEFT] = false;
               break;
            case ALLEGRO_KEY_RIGHT:
               key[KEY_RIGHT] = false;
               break;
            case ALLEGRO_KEY_ESCAPE:
               program_done = true;
               break;
         }
      }

      if (redraw && al_is_event_queue_empty(event_queue))
      {
         redraw = false;

         al_set_target_bitmap(view_port);
         al_clear_to_color(al_map_rgb(0, 0, 0));

         draw_tiles();

         al_set_target_bitmap(game);
         al_draw_bitmap(view_port, 16, 16, 0);
         al_set_target_backbuffer(display);
         al_draw_scaled_bitmap(game, 0, 0, 320, 200, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);

         al_flip_display();
      }
   }

   clean_up();
   return 0;
}
