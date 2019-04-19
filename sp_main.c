#include <stdio.h>
#include <stdarg.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#define TILE001 448, 112 //Macro hack for tile_sheet coordinates... It works, by God!
#define TILE_SIZE 16

const int MAP_WIDTH= 20;
const int MAP_HEIGHT = 20;

const int DISPLAY_WIDTH = 960;
const int DISPLAY_HEIGHT = 600;

const int VIEWPORT_WIDTH = 208;
const int VIEWPORT_HEIGHT = 160;

int CAM_X = 0;
int CAM_Y = 0;

int map[400] = {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
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
//ALLEGRO_EVENT_QUEUE *event_queue = NULL;

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
      jlog("Failed to create display!\n");
      return -1;
   }
   jlog("Display Created.");

   if (!al_init_image_addon())
   {
      jlog("Failed to initialize image_addon!\n");
      return -1;
   }
   jlog("Image addon initialized.");

   //Create viewport bitmap
   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
   stat_border = al_create_bitmap(320, 200);
   game = al_create_bitmap(320, 200);

   tile_sheet = al_load_bitmap("tile_sheet.png");
   if (tile_sheet == NULL) { jlog("Couldn't load tile_sheet.png"); }
   jlog("tile_sheet.png loaded.");


//   //Fill with random 1's and 0's
//   srand(time(NULL));
//   for (int y = 0; y < MAP_WIDTH; y++)
//   {
//      for (int x = 0; x <MAP_HEIGHT; x++)
//      {
//
//         map[y + x * MAP_WIDTH] = rand() % 2;
//      }
//   }




   for (int t = 0; t < MAP_HEIGHT * MAP_WIDTH; t++)
   {
            printf("%d", map[t]);
   }

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
            if ((map[x + y * MAP_WIDTH] != 0))
            {
               al_draw_bitmap_region(tile_sheet, 432, 128, TILE_SIZE, TILE_SIZE, (x * TILE_SIZE) - CAM_X, (y * TILE_SIZE) - CAM_Y, 0); //Draw the tile, subtracting the camera position
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
   //al_destroy_event_queue(event_queue);
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

   //Test code
   al_set_target_bitmap(stat_border);
   al_clear_to_color(al_map_rgb(100, 0, 100));


   for (int i = 0; i < (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT +4; i+=4)
   {
      CAM_Y = i;
      al_rest(0.1);
      al_set_target_bitmap(view_port);
      al_clear_to_color(al_map_rgb(0, 0, 0));
      draw_tiles();
      al_set_target_bitmap(game);
      al_draw_bitmap(stat_border, 0, 0, 0);
      al_draw_bitmap(view_port, 16, 16, 0);

      al_set_target_backbuffer(display);
      al_draw_scaled_bitmap(game, 0, 0, 320, 200, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);

      al_flip_display();
   }

   for (int i = 0; i < (MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH+4; i+=4)
   {
      CAM_X = i;
      al_rest(0.1);
      al_set_target_bitmap(view_port);
      al_clear_to_color(al_map_rgb(0, 0, 0));
      draw_tiles();
      al_set_target_bitmap(game);
      al_draw_bitmap(stat_border, 0, 0, 0);
      al_draw_bitmap(view_port, 16, 16, 0);
      al_set_target_backbuffer(display);
      al_draw_scaled_bitmap(game, 0, 0, 320, 200, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);
      al_flip_display();
   }

   clean_up();
   return 0;
}
