#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "memwatch/memwatch.h"

#include "tt_main.h"
#include "tt_map.h"
#include "tt_player.h"
#include "tt_collision.h"

bool program_done = false;

const float FPS = 16;
const float ANIM_SPEED = 8;

bool redraw = true;

const int DISPLAY_WIDTH = 320 * DISPLAY_MULTIPLIER;
const int DISPLAY_HEIGHT = 200 * DISPLAY_MULTIPLIER;

t_cam cam;
t_map *map = NULL;
t_player player;

bool key[10] = {false};

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *FPS_TIMER = NULL;
ALLEGRO_TIMER *ANIM_TIMER = NULL;
ALLEGRO_MOUSE_STATE mouse;

ALLEGRO_FONT *font = NULL;

//Bitmaps that get loaded from disk
ALLEGRO_BITMAP *tile_sheet = NULL;
ALLEGRO_BITMAP *bg = NULL;
ALLEGRO_BITMAP *stat_border = NULL;
//ALLEGRO_BITMAP *player_start = NULL;

//Bitmaps that get drawn to
ALLEGRO_BITMAP *view_port = NULL;
ALLEGRO_BITMAP *game = NULL;


/*************************************************
 * Initiate everything the game needs at startup *
 *************************************************/
int init_game()
{
   player.cur_frame = 0;
   player.speed = 16;
   player.state = STOPPED;
   player.vel_x = 8;


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
   player.timer = al_create_timer(1.0 / player.speed);
   if(!player.timer)
   {
      jlog("Failed to create Player timer!");
      return -1;
   }
   jlog("Player timer created.");

   if (!al_install_keyboard())
   {
      jlog("Failed to install keyboard!");
      return -1;
   }
   jlog("Keyboard installed.");

   //Font Add-ons
   if(!al_init_font_addon())
   {
      jlog("Failed to install fonts addon!");
      return -1;
   }
   jlog("Fonts add-on installed.");

   //Primitives
   if (!al_init_primitives_addon())
   {
      jlog("Failed to install primitives addon!");
      return -1;
   }
   jlog("Primitives add-on initialized.");

   //Create Display
   //al_set_new_display_flags(ALLEGRO_OPENGL);
   display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
   if(!display)
   {
      jlog("Failed to create display!");
      return -1;
   }
   jlog("Display Created.");

   #ifdef DEBUG
   al_set_window_title(display, "Tally Trauma -- DEBUG");
   #endif // DEBUG

   #ifdef RELEASE
   al_set_window_title(display, "Tally Trauma");
   #endif // RELEASE

   stat_border = al_load_bitmap("data/status_border.png");
   if (stat_border == NULL)
   {
      jlog("Couldn't load status_border.png!");
      return -1;
   }

   al_lock_bitmap(stat_border, al_get_bitmap_format(stat_border), ALLEGRO_LOCK_READONLY);

   bg = al_load_bitmap("data/bg_1.png");
   if (bg == NULL)
   {
      jlog("Couldn't load bg1.png!");
      return -1;
   }
   al_lock_bitmap(bg, al_get_bitmap_format(bg), ALLEGRO_LOCK_READONLY);

   tile_sheet = al_load_bitmap("data/tile_sheet.png");
   if (tile_sheet == NULL)
   {
      jlog("Couldn't load tile_sheet.png!");
      return -1;
   }
   jlog("tile_sheet.png loaded.");
   al_lock_bitmap(tile_sheet, al_get_bitmap_format(tile_sheet), ALLEGRO_LOCK_READONLY);

   player.bitmap = al_load_bitmap("data/player.png");
   if (player.bitmap == NULL) { jlog("Couldn't load player.png"); return -1; }
   al_lock_bitmap(player.bitmap, al_get_bitmap_format(player.bitmap), ALLEGRO_LOCK_READONLY);
   for (int j = 0; j < 8; j++)
   {
      player.frame[j] = al_create_sub_bitmap(player.bitmap, j * 32, 0, 32, 32);
      if (player.frame[j] == NULL)
      {
         jlog("Couldn't create sub-bitmap from player bitmap!");
         return -1;
      }
      al_lock_bitmap(player.frame[j], al_get_bitmap_format(player.frame[j]), ALLEGRO_LOCK_READONLY);
      jlog("Player frame %d created and locked.", j);
   }

   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

   //Create the game bitmap that needs to be stretched to display
   game = al_create_bitmap(320, 200);

   al_register_event_source(event_queue, al_get_display_event_source(display));
   al_register_event_source(event_queue, al_get_timer_event_source(FPS_TIMER));
   al_register_event_source(event_queue, al_get_timer_event_source(ANIM_TIMER));
   al_register_event_source(event_queue, al_get_timer_event_source(player.timer));
   al_register_event_source(event_queue, al_get_keyboard_event_source());

   al_clear_to_color(al_map_rgb(0, 0, 0));

   al_flip_display();

   jlog("Game initialized.");
   return 0;
}

/************************************************
 * The drawing function, called at redraw       *
 ************************************************/
void update_screen()
{

   draw_map(view_port, tile_sheet, bg, &cam, map);
   draw_player(view_port, &cam, &player, player.direction);
   show_player_hotspot(view_port, &cam, &player);

   //Draw view_port to game, then draw game scaled to display.
   al_set_target_bitmap(game);
   al_draw_bitmap(view_port, VIEWPORT_X, VIEWPORT_Y, 0);
   al_set_target_backbuffer(display);
   al_draw_scaled_bitmap(game,
                         0,0,
                         320, 200,
                         0,0,
                         DISPLAY_WIDTH, DISPLAY_HEIGHT,
                         0);
   al_flip_display();
}

/************************************************
 * Makes sure the cam is within the map bounds  *
 ************************************************/
void check_cam() //Check to make sure camera is not out of bounds.
{
   //Check player position and scroll
   if ( (player.x > (VIEWPORT_WIDTH / 2) -24) && (player.x < (MAP_WIDTH * TILE_SIZE) - (VIEWPORT_WIDTH / 2))  )
   {
      cam.x = player.x - ((VIEWPORT_WIDTH / 2) -16);
   }
   if ( (player.y > (VIEWPORT_HEIGHT / 2) - 24) && (player.y < (MAP_HEIGHT * TILE_SIZE) - (VIEWPORT_HEIGHT / 2)) )
   {
      cam.y = player.y - ((VIEWPORT_HEIGHT / 2) -16);
   }
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
      case ALLEGRO_KEY_LCTRL:
         key[KEY_LCTRL] = true;
         break;
      case ALLEGRO_KEY_LSHIFT:
         key[KEY_LSHIFT] = true;
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
      case ALLEGRO_KEY_LCTRL:
         key[KEY_LCTRL] = false;
         break;
      case ALLEGRO_KEY_LSHIFT:
         key[KEY_LSHIFT] = false;
         break;
      case ALLEGRO_KEY_ESCAPE:
         program_done = true;
         break;
   }
}

/************************************************
 * Updated the player and movements             *
 ************************************************/
void update_player()
{
   int old_x = player.x;
   int x1, x2;

   if (player.direction == LEFT)
   {
      x1 = 17;
      x2 = 19;
   }
   else
   {
      x1 = 14;
      x2 = 13;
   }

   //Horizontal Movement
   if (key[KEY_RIGHT] && !key[KEY_LEFT])
      {
         player.direction = RIGHT;
         player.x += player.vel_x;
         if (player.on_ground) player.state = WALKING;
      }
      else if (key[KEY_LEFT] && !key[KEY_RIGHT])
      {
         if (player.on_ground) player.state = WALKING;
         player.direction = LEFT;
         player.x -= player.vel_x;
      }
      else if (!key[KEY_LEFT] && !key[KEY_RIGHT])
      {
         if (player.on_ground) player.state = STOPPED;
      }
   //Horizontal tile collision
   if (is_ground(map, player.x + 16, player.y +1)) player.x = old_x;
   if (is_ground(map, player.x + 8, player.y +1)) player.x = old_x;
   if (is_ground(map, player.x + 16, player.y +31)) player.x = old_x;
   if (is_ground(map, player.x +8, player.y +31)) player.x = old_x;

   //Vertical Movement
   if (!is_ground(map, player.x + x1, player.y +32))
   {
      if(!is_ground(map, player.x +x2, player.y + 32))
      {
         player.vel_y += 16;
         player.on_ground = false;
         if (player.vel_y >= 0) player.state = FALLING;
         if (player.vel_y < 0) player.state = JUMPING;
      }
   }
   else
   {
      player.on_ground = true;
      player.vel_y = 0;
      player.jump_pressed = false;
      player.jumping = false;
   }

   //Pressed jump
   if (key[KEY_LCTRL] && player.on_ground && !player.jump_pressed && !player.jumping)
   {

      player.vel_y = -82;
      player.jumping = true;
      player.jump_pressed = true;
   }
   //Apply vertical force
   player.y += player.vel_y >> 2;
   if (player.vel_y > 32) player.vel_y = 32;

   //Check floor
   while (is_ground(map, player.x + x1, player.y +31)) {player.y--; player.jumping = false; player.on_ground = true; }
   while (is_ground(map, player.x + x2, player.y +31)) {player.y--; player.jumping = false; player.on_ground = true; }

   //Check roof
   while (is_ground(map, player.x + x1, player.y)) {player.y++; player.vel_y = 0;}
   while (is_ground(map, player.x + x2, player.y)) {player.y++; player.vel_y = 0;}

   printf("player.tate: %d\n", player.state);
   //printf("player.vel_y: %d\n", player.vel_y);
}

/************************************************
 * Checks logic that needs to be timed by FPS   *
 ************************************************/
void check_timer_logic(ALLEGRO_EVENT *ev)
{
   if (ev->timer.source == player.timer)
   {
      /**** Player movement ****/
      update_player();
      animate_player(&player);
   }
   //Frames
   if (ev->timer.source == FPS_TIMER)
   {

   }



   //Animation
   if (ev->timer.source == ANIM_TIMER)
   {

   }




   check_cam();
}

/************************************************
 * Clean-ups for end of program                 *
 ************************************************/
void clean_up()
{
   destroy_map(map);
   al_unlock_bitmap(tile_sheet);
   al_unlock_bitmap(bg);
   al_unlock_bitmap(stat_border);
   al_unlock_bitmap(player.bitmap);
   for (int j = 0; j < 7; j++)
   {
      al_unlock_bitmap(player.frame[j]);
   }

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

   if (init_game() != 0)
   {
      jlog("Failed to init game!\n");
      return -1;
   }

   map = load_map("data/maps/map.spl"); //Create an empty map
   if (map == NULL)
   {
      jlog("In file %s, Line %d. Couldn't create an empty map!" __FILE__, __LINE__);
      return -1;
   }
   jlog("Map loaded.");
   player.x = map->player_start_x;
   player.y = map->player_start_y;
   check_cam();

   al_set_target_bitmap(game);
   al_draw_bitmap(stat_border, 0, 0, 0);

   //This is the main loop for now

   al_start_timer(FPS_TIMER);
   jlog("FPS timer started.");
   al_start_timer(ANIM_TIMER);
   jlog("Animation timer started.");
   al_start_timer(player.timer);
   jlog("Player timer started.");
   while(!program_done)
   {

      ALLEGRO_EVENT ev;
      al_wait_for_event(event_queue, &ev);

      if (ev.type == ALLEGRO_EVENT_TIMER)
      {
         check_timer_logic(&ev);
         redraw = true;
      }
      else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
      {
         program_done = true;
      }

      if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
      {
         check_key_down(&ev);
      }
      else if (ev.type == ALLEGRO_EVENT_KEY_UP)
      {
         check_key_up(&ev);
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

