#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "tt_init.h"
#include "tt_main.h"
#include "tt_thing.h"
#include "tt_map.h"
#include "tt_player.h"
#include "tt_collision.h"
#include "tt_items.h"
#include "tt_bullet.h"

#define BUFFER_SAMPLES 2048
#define STREAM_FREQ 44100

//int frames = 0;
//int minutes = 0;
//int seconds = 0;
//int centiseconds = 0;

/* At some point I'll see if I can prune these globals, but they're staying for now. */
int demo_file_pos = 0;

t_game game = { .state = LOAD_LEVEL, .next_state = LOAD_LEVEL };

const float FPS = 30;
int frame_speed = ANIMATION_SPEED;
int halftime_frame_speed = ANIMATION_SPEED * 2;

t_bullet player_bullet;

t_screen screen = { .unscaled_w = 320, .unscaled_h = 200 };

t_cam cam;

t_map *map = NULL;
t_player player = { .cur_frame = 0, .state = STOPPED, .vel_x = 4 };

t_item_afterfx *item_fx = NULL;

int screen_flash = -1;

t_thing thing[MAX_THINGS];

unsigned char item_frame = 0;
unsigned char item_afterfx_frame = 0;

bool key[6] = { false };
enum KEYS {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_JUMP, KEY_FIRE};

#define ORLO_BUFFER_SIZE 32 

typedef struct
{
   int x;
   int y;
   unsigned char state;
   unsigned char direction;
   unsigned char cur_frame;
} t_orlo_position_buffer;

t_orlo_position_buffer orlo_position_buffer[ORLO_BUFFER_SIZE] = { 0 };
t_thing orlo = { 0 };
bool orlo_gave_health = false;
int orlo_gave_health_life_span = 0;
int orlo_message_lifetime = -1;
int orlo_message_to_show = 0;
#define MSG_NONE        0
#define MSG_GET_HEALTH  1
#define MSG_GIVE_HEALTH 2
const char ORLO_TXT_GET_HEALTH[34] = "I acquired health for you.";
const char ORLO_TXT_GIVE_HEALTH[21] = "Health administered.";

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *FPS_TIMER = NULL;
ALLEGRO_MOUSE_STATE mouse;

ALLEGRO_FONT *font_status = NULL;
ALLEGRO_FONT *font_message = NULL;

//Bitmaps that get loaded from disk
t_graphics graphics = { NULL };

//Bitmaps that get drawn tom
ALLEGRO_BITMAP *view_port = NULL;
ALLEGRO_BITMAP *game_bmp = NULL;

//Sounds
float sfx_volume = 1.0;
t_sounds sounds = { NULL };

ALLEGRO_SAMPLE_ID *snd_jump_id = NULL;

ALLEGRO_AUDIO_STREAM *music_stream = NULL;
struct ADL_MIDIPlayer *midi_player = NULL;

short *opl_buffer = NULL;
int samples_count = 0;

/************************************************
 * Makes sure the cam is within the map bounds  *
 ************************************************/
void check_cam() //Check to make sure camera is not out of bounds.
{

   //Check player position and scroll
   if ( (player.x > (VIEWPORT_WIDTH / 2) -24) && (player.x < (MAP_WIDTH * TILE_SIZE) - (VIEWPORT_WIDTH / 2))  )
   {
      cam.x = player.x - ((VIEWPORT_WIDTH / 2) -16) + cam.look_ahead;
   }

   //Scroll when player is on ground or leaving view port.
   //On ground
   if ( player.on_ground && (cam.y < player.y - (VIEWPORT_HEIGHT / 2) -16) )
   {
      cam.y += 8;
   }
   if ( player.on_ground && (cam.y > player.y - (VIEWPORT_HEIGHT / 2) + 16) )
   {
      cam.y -= 8;
   }
   //Leaving view port
   if ( !player.on_ground && player.y < cam.y +8)
   {
      cam.y = player.y - 8;
   }
   if ( !player.on_ground && player.y + 32 > ((cam.y + (VIEWPORT_HEIGHT - 8))) )
   {
      cam.y = ((player.y + 32) - VIEWPORT_HEIGHT + 8);
   }

   if (cam.x < 0) cam.x = 0;
   if (cam.x > (MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH)
   {
      cam.x = ((MAP_WIDTH * TILE_SIZE) - VIEWPORT_WIDTH);
   }

   if (cam.y < 0) cam.y = 0;
   if (cam.y > (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT)
   {
      cam.y = (MAP_HEIGHT * TILE_SIZE) - VIEWPORT_HEIGHT;
   }
}

/*
 * Loads a level up at LOAD_LEVEL game state
 */
 bool load_level(char *map_file, char *music_file)
 {
   //Display loading image
   al_draw_scaled_bitmap(graphics.loading, 0, 0, screen.unscaled_w, screen.unscaled_h, screen.x, screen.y, screen.width, screen.height, 0);
   al_flip_display();
   //al_rest(3);  //Enable this when screen recording for time to hit the record button!
   //Load map
   map = load_map(map_file); //Create an empty map
   if (map == NULL)
   {
      jlog("In file %s, Line %d. Couldn't create an empty map!" __FILE__, __LINE__);
      return false;
   }
   jlog("Map loaded.");

   //Set Orlo stuff
   orlo_position_buffer[0].x = player.x;
   orlo_position_buffer[0].y = player.y;
   orlo.bb_height = 32;
   orlo.bb_width = 12;
   orlo.bb_top = 0;
   orlo.bb_left = 9;
   orlo.width = 32;
   orlo.height = 32;
   orlo.state = ORLO_STATE_NORMAL;

   //Set player stuff
   player.x = map->player_start_x;
   player.y = map->player_start_y;
   player.bb_width = 8;
   player.bb_height = 28;
   player.draw = true;
   player.health = 8;

   //Check cam position
   check_cam();

   //Create item after effects for items in map
   item_fx = create_item_after_fx(map);

   //Load the graphics for a level
   graphics.stat_border = al_load_bitmap("data/stat_border.png");
   if (graphics.stat_border == NULL)
   {
      jlog("Couldn't load stat_border.png!");
      return false;
   }

   graphics.bg = al_load_bitmap("data/bg_1.png");
   if (graphics.bg == NULL)
   {
      jlog("Couldn't load bg1.png!");
      return false;
   }

   graphics.tile_sheet = al_load_bitmap("data/tile_sheet.png");
   if (graphics.tile_sheet == NULL)
   {
      jlog("Couldn't load tile_sheet.png!");
      return false;
   }

   graphics.item_sheet = al_load_bitmap("data/item_sheet.png");
   if (graphics.tile_sheet == NULL)
   {
      jlog("Couldn't load item_sheet.png!");
      return false;
   }
   jlog("item_sheet.png loaded.");

   player.bitmap = al_load_bitmap("data/player.png");
   if (player.bitmap == NULL) { jlog("Couldn't load player.png"); return -1; }

   for (int j = 0; j < 8; j++)
   {
      player.frame[j] = al_create_sub_bitmap(player.bitmap, j * 32, 0, 32, 32);
      if (player.frame[j] == NULL)
      {
         jlog("Couldn't create sub-bitmap from player bitmap!");
         return false;
      }
      jlog("Player frame %d created.", j);
   }

   orlo.bitmap = al_load_bitmap("data/orlo.png");
   if (orlo.bitmap == NULL) { jlog("Couldn't load orlo.png"); return -1; }
   for (int i =0; i < 8; i++)
   {
      orlo.frame[i] = al_create_sub_bitmap(orlo.bitmap, i * 32, 0, 32, 32);
      if (orlo.frame[i] == NULL)
      {
         jlog("Couldn't create sub-bitmap from orlo bitmap!");
         return false;
      }
      jlog("Orlo frame %d created.", i);
   }

   graphics.bad_robot_1 = al_load_bitmap("data/badrobot.png");
   if (graphics.bad_robot_1 == NULL) { jlog("Couldn't load badrobot.png"); return false; }

   graphics.item_fx_sheet = al_load_bitmap("data/item_score.png");
   if (graphics.item_fx_sheet == NULL) { jlog("Couldn't load item_score.png"); return false; }
  
   graphics.health_bar = al_load_bitmap("data/health_bar.png");
   if (graphics.health_bar == NULL) { jlog("Couldn't load health_bar.png"); return false; }

   graphics.muzzle_flash = al_load_bitmap("data/muzzle_flash.png");
   if (graphics.muzzle_flash == NULL) { jlog("Couldn't load muzzle_flash.png"); return false; }
   
   graphics.bullet_particle = al_load_bitmap("data/particle.png");
   if (graphics.bullet_particle == NULL) { jlog("Couldn't load particle.png"); return false; }

   graphics.laser = al_load_bitmap("data/laser.png");
   if (graphics.laser == NULL) { jlog("Couldn't load laser.png"); return false; }

   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

   //Load sounds
   sounds.fall = al_load_sample("data/sound/fall.wav");
   sounds.jump = al_load_sample("data/sound/jump.wav");
   sounds.land = al_load_sample("data/sound/land.wav");
   sounds.hithead = al_load_sample("data/sound/hithead.wav");
   sounds.pickup = al_load_sample("data/sound/pickup.wav");
   sounds.health = al_load_sample("data/sound/health.wav");
   sounds.hurt = al_load_sample("data/sound/hurt.wav");
   sounds.shoot = al_load_sample("data/sound/shoot.wav");
   sounds.orlo_give_health = al_load_sample("data/sound/orlogivehealth.wav");
   sounds.orlo_get_health = al_load_sample("data/sound/orloacquiredhealth.wav");

   al_set_audio_stream_playing(music_stream, true);

   //Load things
   load_things(thing, map);

   //draw the status border
   al_set_target_bitmap(game_bmp);
   al_draw_bitmap(graphics.stat_border, 0, 0, 0);

   game.level_needs_unloaded = true;

   //centiseconds = 0;
   //seconds = 0;
   //minutes = 0;
   return true; //Returns true on success
 }

/*
 * Unloads a level
 */
bool unload_level()
{
   destroy_map(map);
   map = NULL;
   if (map == NULL) jlog("map unloaded.");

   destroy_item_afterfx(item_fx);
   item_fx = NULL;
   if (item_fx == NULL) jlog("item_fx unloaded.");

   al_destroy_bitmap(graphics.tile_sheet);
   graphics.tile_sheet = NULL;
   if (graphics.tile_sheet == NULL) jlog("tile_sheet unloaded.");
   al_destroy_bitmap(graphics.item_sheet);
   graphics.item_sheet = NULL;
   if (graphics.item_sheet == NULL) jlog("item_sheet unloaded.");

   al_destroy_bitmap(graphics.bg);
   graphics.bg = NULL;
   if (graphics.bg == NULL) jlog("bg unloaded.");

   al_destroy_bitmap(graphics.stat_border);
   graphics.stat_border = NULL;
   if (graphics.stat_border == NULL) jlog("stat_border unloaded.");

   al_destroy_bitmap(graphics.item_fx_sheet);
   graphics.item_fx_sheet = NULL;
   if (graphics.item_fx_sheet == NULL) jlog("item_fx_sheet unloaded.");

   al_destroy_bitmap(graphics.health_bar);
   graphics.health_bar = NULL;
   if (graphics.health_bar == NULL) jlog("health_bar unloaded.");

   al_destroy_bitmap(graphics.muzzle_flash);
   graphics.muzzle_flash = NULL;
   if (graphics.muzzle_flash == NULL) jlog("muzzle_flash unloaded.");

   al_destroy_bitmap(graphics.bullet_particle);
   graphics.bullet_particle = NULL;
   if (graphics.bullet_particle == NULL) jlog("bullet_particle unloaded.");

   al_destroy_bitmap(graphics.laser);
   graphics.laser = NULL;
   if (graphics.laser == NULL) jlog("laser unloaded.");

   for (int j = 0; j < 8; ++j)
   {
      al_destroy_bitmap(player.frame[j]);
      player.frame[j] = NULL;
      if (player.frame[j] == NULL) jlog("player.frame[%d] unloaded.", j);
   }

   al_destroy_bitmap(player.bitmap);
   player.bitmap = NULL;
   if (player.bitmap == NULL) jlog("player.bitmap unloaded.");


   al_destroy_bitmap(view_port);
   view_port = NULL;
   if (view_port == NULL) jlog("view_port unloaded.");

   al_destroy_sample(sounds.jump);
   sounds.jump = NULL;

   al_destroy_sample(sounds.land);
   sounds.land = NULL;

   al_destroy_sample(sounds.hithead);
   sounds.hithead = NULL;

   al_destroy_sample(sounds.pickup);
   sounds.pickup = NULL;

   al_destroy_sample(sounds.health);
   sounds.health = NULL;

   al_destroy_sample(sounds.hurt);
   sounds.hurt = NULL;

   al_destroy_sample(sounds.shoot);
   sounds.shoot = NULL;

   game.level_needs_unloaded = false;
   return true; //Returns true on success
}


/*
 * Plays a sound without all the boilerplate
 */
void play_sound(ALLEGRO_SAMPLE *s, bool interupt)
{
   if (interupt) al_stop_samples();
   al_play_sample(s, sfx_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
}

/*
 * Draw laser
 */
void draw_laser()
{
   if (player.direction == RIGHT)
   {
      for (int x = player_bullet.start_x; x < player_bullet.end_x; x++)
      {
         al_draw_bitmap(graphics.laser, x - cam.x , player.muzzle_y - cam.y + 6, 0);
      }
   }

   if (player.direction == LEFT)
   {
      for (int x = player_bullet.start_x; x > player_bullet.end_x; x--)
      {
          al_draw_bitmap(graphics.laser, x - cam.x , player.muzzle_y - cam.y + 6, 0);
      }
   }
}

/*
 * Checks "Key Down" events
 */
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
         key[KEY_JUMP] = true;
         break;
      case ALLEGRO_KEY_X:
         key[KEY_FIRE] = true;
         break;
   }
}

/*
 * Checks "Key Up" events
 */
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
         key[KEY_JUMP] = false;
         break;
      case ALLEGRO_KEY_X:
         key[KEY_FIRE] = false;
         break;
      case ALLEGRO_KEY_ENTER: //Tempory code to switch to fullscreen at runtime
         screen.width = 960;
         screen.height = 600;
         al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, true);
         screen.factor_x = al_get_display_width(display) / screen.unscaled_w;
         screen.factor_y = al_get_display_height(display) / screen.unscaled_h;
         screen.factor = (screen.factor_y < screen.factor_x) ? screen.factor_y : screen.factor_x;
         screen.width = screen.unscaled_w * screen.factor;
         screen.height = screen.unscaled_h * screen.factor;
         screen.x = (al_get_display_width(display) / 2) - (screen.width/2);
         screen.y = (al_get_display_height(display) / 2) - (screen.height/2);
         /* for (int y = 0; y < (al_get_display_height(display) / (32 * screen.factor)) + 1; y++)
         {
            for (int x = 0; x < (al_get_display_width(display) / (32 * screen.factor)) + 1; x++)
            {
               al_draw_scaled_bitmap(border, 0, 0, 32, 32, x * (32 * screen.factor), y * (32 * screen.factor), 32 * screen.factor, 32 * screen.factor, 0);
            }
         } */
         jlog("Display Created.");
         break;

      //Reset player to start point. (Debug key)
      case ALLEGRO_KEY_P:
         player.x = map->player_start_x;
         player.y = map->player_start_y;
         cam.x = player.x - VIEWPORT_WIDTH / 2 + 24;
         cam.y = player.y - VIEWPORT_HEIGHT / 2 + 16;
         cam.look_ahead = 0;
         check_cam();
         break;

      case ALLEGRO_KEY_ESCAPE:
         game.state = QUIT;
         break;
   }
}

/*
 * Check bullet collisions
 */
bool check_bullet_collision()
{
   if (player.direction == RIGHT)
   {
      player_bullet.start_x = player.x + 25;
      player_bullet.start_y = player.y + 18;
      for (int x = player_bullet.start_x; x < cam.x + VIEWPORT_WIDTH + 16; x++)
      {
         if (is_ground(map, x, player_bullet.start_y))
         {
            player_bullet.end_x = x;
            player_bullet.end_y = player_bullet.start_y;
            jlog("Bullet hit at %d, %d", player_bullet.end_x, player_bullet.end_y);
            player_bullet.draw = true;
            return true;
         }
      }
   }

   if (player.direction == LEFT)
   {
      player_bullet.start_x = player.x + 6;
      player_bullet.start_y = player.y + 18;
      for (int x = player_bullet.start_x; x > cam.x - 16; x--)
      {
         if (is_ground(map, x, player_bullet.start_y))
         {
            player_bullet.end_x = x;
            player_bullet.end_y = player_bullet.start_y;
            jlog("Bullet hit at %d, %d", player_bullet.end_x, player_bullet.end_y);
            player_bullet.draw = true;
            return true;
         }
      }
   }

   if (player.direction == RIGHT)
   {
      player_bullet.start_x = player.x + 25;
      player_bullet.start_y = player.y + 18;
      player_bullet.end_x = player_bullet.start_x + ((cam.x + VIEWPORT_WIDTH) - player_bullet.start_x) + 16;
      player_bullet.end_y = player_bullet.start_y;

   }
   if (player.direction == LEFT)
   {
      player_bullet.start_x = player.x + 6;
      player_bullet.start_y = player.y + 18;
      player_bullet.end_x = player_bullet.start_x - (cam.x + player_bullet.start_x) -16;
      player_bullet.end_y = player_bullet.start_y;
   }

   player_bullet.draw = false;
   return false;
}

/*
 * Update the player and movements
 */
void update_player()
{
   int old_x = player.x;
   int x1, x2, x3;
   int tx, ty, ty2, ty3;
   t_map_pos *mp;
   t_map_pos *mp2;
   t_map_pos *mp3;
   bool landed = false;

   player.bb_top = 4;

   //Horizontal Movement
   if (key[KEY_RIGHT] && !key[KEY_LEFT])
   {
      player.direction = RIGHT;
      player.x += player.vel_x;
      if (player.on_ground) player.state = WALKING;
   }
   else if (key[KEY_LEFT] && !key[KEY_RIGHT])
   {
      player.direction = LEFT;
      player.x -= player.vel_x;
      if (player.on_ground) player.state = WALKING;
   }
   else if (!key[KEY_LEFT] && !key[KEY_RIGHT])
   {
      if (player.on_ground) player.state = STOPPED;
   }
   else if (key[KEY_LEFT] && key[KEY_RIGHT] && player.on_ground)
   {
      if (player.on_ground) player.state = STOPPED;
   }

   /* These look like magic numbers, but they have
   to be fairly precise, or else when the player
   is standing on the edge of a tile the velocity
   gets wacky, so these points can't hang off the
   edge of a tile. Remember, This game's player moves
   4 pixels at a time. So if at some point one of
   the points is off the edge, the vertical movement
   check will still increase the velocity, but the
   player won't move. */
   if (player.direction == RIGHT)
   {
      x1 = 19;
      x2 = 11;
      x3 = 15; //For detecting falling from edge.
      player.bb_left = 12;
   }
   else
   {
      x1 = 14;
      x2 = 22;
      x3 = 18; //For detecting falling from edge.
      player.bb_left = 12;
   }

   /* Horizontal Tile Collision
      Basically checks three points in the player's
      Y position for each side of the player. If there's
      something there it sets the player's position back
      to its previous position before the collision occurred.
      But because we're not drawing here, it doesn't show the
      jerkiness of the process. */
   if (player.y + 31 < MAP_HEIGHT * TILE_SIZE)
   {
      if (player.y + 32 > 0)
      {
         if (is_ground(map, player.x + x1, player.y + 2 )) player.x = old_x; //top
         if (is_ground(map, player.x + x2, player.y + 2 )) player.x = old_x;

         if (is_ground(map, player.x + x1, player.y + 16 )) player.x = old_x; //center
         if (is_ground(map, player.x + x2, player.y + 16 )) player.x = old_x;

         if (is_ground(map, player.x + x1, player.y + 31 )) player.x = old_x; //bottom
         if (is_ground(map, player.x + x2, player.y + 31 )) player.x = old_x;
      }
   }
   /* Vertical movement
      Checks the two points below the player's feet
      if there's not a solid tile, it adds 4 to
      the Y velocity. Otherwise the player is standing
      on a solid tile. */
   if ( (!is_ground(map, player.x + x1, player.y + 32) && !is_ground(map, player.x +x2, player.y + 32)) )
   {
      player.vel_y += 4;
      player.on_ground = false;
      if (player.vel_y >= 0) player.state = FALLING;
      if (player.vel_y < 0) player.state = JUMPING;
   }
   else
   {
      if (player.vel_y > 0) landed = true; //So we can make a landing sound.
      player.on_ground = true;
      //player.jump_pressed = false;
      player.vel_y = 0;

   }

   if (landed == true)
   {
      if (is_ground(map, player.x + x1, player.y + 32))
      {
         landed = false;
         play_sound(sounds.land, false);
      }
   }

   /* I added this in hopes that we could detect
   when a player falls off edge. Working so far.
   also detects if player barely lands on ledge
   and helps them out a little. */
   if(player.on_ground == true && !is_ground(map, player.x + x1, player.y + 32) && !is_ground(map, player.x +x3, player.y + 32) && !key[KEY_JUMP] && player.y + 31 > 0)
   {
      //Play fall off ledge sound
      player.state = FALLING;
      if (player.direction == RIGHT) player.x += 4;
      if (player.direction == LEFT) player.x -= 4;
      player.vel_y += 24;
      play_sound(sounds.fall, false);
   }
   else if(player.on_ground == true && !is_ground(map, player.x + x1, player.y + 32) && !is_ground(map, player.x +x3, player.y + 32) && key[KEY_JUMP] && player.y + 31 > 0)
   {
      //Play fall off ledge sound
      if (player.direction == RIGHT) player.x += 4;
      if (player.direction == LEFT) player.x -= 4;
      //player.vel_y = -24;
   }
   else if(player.on_ground == true && is_ground(map, player.x + x1, player.y + 32) && !is_ground(map, player.x +x3, player.y + 32) && player.y + 31 > 0)
   {
      if (player.direction == RIGHT) player.x += 4;
      if (player.direction == LEFT) player.x -= 4;
   }

   /* Jump button is pressed
      If the player is on the ground, and jump
      is pressed. The player jumps. Subtracting
      a number from the velocity, this gives the player
      a vertical upwards boost while the Y velocity is
      still being pulled in the opposite direction,
      simulating gravity. */
      if (key[KEY_JUMP] && player.on_ground && !player.jump_pressed)
      {
         if (player.y + 1 > 0 && !is_ground(map, player.x + x1, player.y-1) && !is_ground(map, player.x + x2, player.y-1))
         {
            if (!is_ground(map, player.x + x1, player.y-1) && !is_ground(map, player.x + x2, player.y-1) && player.y + 31 > 0)
            {
               play_sound(sounds.jump, false);
            }
            player.vel_y = -46;
            player.jump_pressed = true;
            player.on_ground = false;
         }
         else if (player.y + 1 < 0 && is_ground(map, player.x + x1, player.y-1) && is_ground(map, player.x + x2, player.y-1))
         {
            if (is_ground(map, player.x + x1, player.y-1) && is_ground(map, player.x + x2, player.y-1) && player.y + 31 > 0)
            {
               play_sound(sounds.jump, false);
            }
            player.vel_y = -46;
            player.jump_pressed = true;
            player.on_ground = false;
         }
      }

   if (!key[KEY_JUMP])
   {
      player.jump_pressed = false;
      if (player.vel_y < 0) player.vel_y /= 2;
   }

   /* Apply vertical force
      This is where the player's Y position is changed
      based on it's Y velocity divided by 4. Effectively
      pulling the player back down if it's in the air.
      But it's capped so it doesn't get infinitely faster. */
   player.y += player.vel_y / 4;
   if (player.vel_y > 48) player.vel_y = 48;

   /* Check floor
      If the player is literally inside a tile, this
      starts pulling them out until it's free of the tile.
      And since it's a loop, and we're not drawing it
      here, we don't actually see what is happening. */
   if (player.y + 31 < MAP_HEIGHT * TILE_SIZE)
   {
      while (is_ground(map, player.x + x1, player.y + 31) && player.y + 31 > 0)
      {
         player.y--;
         player.on_ground = true;
      }
      while (is_ground(map, player.x + x2, player.y + 31) && player.y + 31 > 0)
      {
         player.y--;
         player.on_ground = true;
      }
   }

   /* Check Ceiling
      Same as above except at the players top. */
   if (player.y > 0)
   {
      while (is_ground(map, player.x + x1, player.y))
      {
         player.y++;
         player.vel_y = 0;
         if(player.state == JUMPING)
         {
            play_sound(sounds.hithead, false);
         }
         player.state = FALLING;

      }
      while (is_ground(map, player.x + x2, player.y))
      {
         player.y++;
         player.vel_y = 0;
         if(player.state == JUMPING)
         {
            play_sound(sounds.hithead, false);
         }
         player.state = FALLING;
      }
   }
   
   
   /* Check for items
      Here we check if the player is over an item
      This get a little redundant. */
   tx = player.x + (player.direction ? 14 : 18);
   /* There are three of each of these so I can check
      Tally's head, feet, and middle. */
   ty = (player.y + 1);
   ty2 = (player.y + 31);
   ty3 = (player.y + 16);
   mp = get_map_position(map, tx, ty);
   mp2 = get_map_position(map, tx, ty2);
   mp3 = get_map_position(map, tx, ty3);

   if (mp != NULL && mp2 != NULL)
   {
      if (mp->item > 0 || mp2->item > 0 || mp3->item > 0)
      {
         //Burger
         if (mp->item == ITEM_BURGER) { mp->item = 0; activate_item_fx(mp, item_fx); player.score += 100; if (player.health < 8) player.health++; jlog("Health: %d", player.health); }
         if (mp2->item == ITEM_BURGER) { mp2->item = 0; activate_item_fx(mp2, item_fx); player.score += 100; if (player.health < 8) player.health++; jlog("Health: %d", player.health); }
         if (mp3->item == ITEM_BURGER) { mp3->item = 0; activate_item_fx(mp3, item_fx); player.score += 100; if (player.health < 8) player.health++; jlog("Health: %d", player.health); }

         //Disk
         if (mp->item == ITEM_DISK) { mp->item = 0; activate_item_fx(mp, item_fx); player.score += 100; }
         if (mp2->item == ITEM_DISK) { mp2->item = 0; activate_item_fx(mp2, item_fx); player.score += 100; }
         if (mp3->item == ITEM_DISK) { mp3->item = 0; activate_item_fx(mp3, item_fx); player.score += 100; }

         //VHS
         if (mp->item == ITEM_VHS) { mp->item = 0; activate_item_fx(mp, item_fx); player.score += 200; }
         if (mp2->item == ITEM_VHS) { mp2->item = 0; activate_item_fx(mp2, item_fx); player.score += 200; }
         if (mp3->item == ITEM_VHS) { mp3->item = 0; activate_item_fx(mp3, item_fx); player.score += 200; }

         //Screw
         if (mp->item == ITEM_SCREW) { mp->item = 0; activate_item_fx(mp, item_fx); player.score += 200;}
         if (mp2->item == ITEM_SCREW) { mp2->item = 0; activate_item_fx(mp2, item_fx); player.score += 200; }
         if (mp3->item == ITEM_SCREW) { mp3->item = 0; activate_item_fx(mp3, item_fx); player.score += 200; }

         //Underwear
         if (mp->item == ITEM_UNDERWEAR) { mp->item = 0; activate_item_fx(mp, item_fx); player.score += 500; }
         if (mp2->item == ITEM_UNDERWEAR) { mp2->item = 0; activate_item_fx(mp2, item_fx); player.score += 500; }
         if (mp3->item == ITEM_UNDERWEAR) { mp3->item = 0; activate_item_fx(mp3, item_fx); player.score += 500; }

         //Pliers
         if (mp->item == ITEM_PLIERS) { mp->item = 0; activate_item_fx(mp, item_fx); player.score += 500; }
         if (mp2->item == ITEM_PLIERS) { mp2->item = 0; activate_item_fx(mp2, item_fx); player.score += 500; }
         if (mp3->item == ITEM_PLIERS) { mp3->item = 0; activate_item_fx(mp3, item_fx); player.score += 500; }

         //Wrench
         if (mp->item == ITEM_WRENCH) { mp->item = 0; activate_item_fx(mp, item_fx); player.score += 1000; }
         if (mp2->item == ITEM_WRENCH) { mp2->item = 0; activate_item_fx(mp2, item_fx); player.score += 1000; }
         if (mp3->item == ITEM_WRENCH) { mp3->item = 0; activate_item_fx(mp3, item_fx); player.score += 1000; }

         //Screwdriver
         if (mp->item == ITEM_SCREWDRIVER) { mp->item = 0; activate_item_fx(mp, item_fx); player.score += 1000; }
         if (mp2->item == ITEM_SCREWDRIVER) { mp2->item = 0; activate_item_fx(mp2, item_fx); player.score += 1000; }
         if (mp3->item == ITEM_SCREWDRIVER) { mp3->item = 0; activate_item_fx(mp3, item_fx); player.score += 1000; }

         //Money
         if (mp->item == ITEM_MONEY) { mp->item = 0; activate_item_fx(mp, item_fx); player.score += 2000; }
         if (mp2->item == ITEM_MONEY) { mp2->item = 0; activate_item_fx(mp2, item_fx); player.score += 2000; }
         if (mp3->item == ITEM_MONEY) { mp3->item = 0; activate_item_fx(mp3, item_fx); player.score += 2000; }

         //Diamond
         if (mp->item == ITEM_DIAMOND) { mp->item = 0; activate_item_fx(mp, item_fx); player.score += 5000; }
         if (mp2->item == ITEM_DIAMOND) { mp2->item = 0; activate_item_fx(mp2, item_fx); player.score += 5000; }
         if (mp3->item == ITEM_DIAMOND) { mp3->item = 0; activate_item_fx(mp3, item_fx); player.score += 5000; }

         // Health
         if (mp->item == ITEM_HEALTH && player.health < 8)
         {
            player.health = 8;
            mp->item = 0;
            activate_item_fx(mp, item_fx);
            jlog("Health: %d", player.health);
            play_sound(sounds.health, false);
            player.score += 100;
         }
         else if (mp2->item == ITEM_HEALTH && player.health < 8)
         {
            player.health = 8;
            mp2->item = 0;
            activate_item_fx(mp2, item_fx);
            jlog("Health: %d", player.health);
            play_sound(sounds.health, false);
            player.score += 100;
         }
         else if (mp3->item == ITEM_HEALTH && player.health < 8)
         {
            player.health = 8;
            mp3->item = 0;
            activate_item_fx(mp3, item_fx);
            jlog("Health: %d", player.health);
            play_sound(sounds.health, false);
            player.score += 100;
         }
         // Not-Health sound
         else if (mp->item != ITEM_HEALTH && mp2->item != ITEM_HEALTH && mp3->item != ITEM_HEALTH)
         {
            play_sound(sounds.pickup, false);
            jlog("Score: %d", player.score);
         }
      }
   }


   //Collisions against things
   for (int i = 0; i < map->num_things; i++)
   {
      if (collision_check(&player, &thing[i]))
      {
         if (!player.hurt && thing[i].type < 10 && player.health) //If thing can hurt player (The first 10 types can)
         {
            play_sound(sounds.hurt, false);
            player.hurt = PLAYER_HURT_TIME;
            if(player.health) player.health--;
            screen_flash = 2;
         }
         if (thing[i].type == ENEMY_BAD_ROBOT)
         {
            if (thing[i].touched == false) //play_sound(orlo_give_health, false);
            thing[i].touched = true;
         }
      }
   }
   if (player.hurt)
   {
      player.hurt--;
   }

   //Shooting time
   if (key[KEY_FIRE] && !player.shoot_time)
   {
      play_sound(sounds.shoot, false);
      player.shoot_time = 5;
      player.muzzle_time = 3;
   }
   if (player.muzzle_time) player.muzzle_time--;
   else player_bullet.draw = false;

   if (player.shoot_time == 5) check_bullet_collision();
   if (player.shoot_time) player.shoot_time--;

   if (screen_flash > -1) screen_flash--;
}

/*
 * Checks logic that needs to be timed by FPS
 */
void check_timer_logic()
{
   
   //Frames
   frame_speed--; //This eliminates the need for an animation timer.
   halftime_frame_speed--; //This is for 2 frame animations like for blinking items

   update_player();
   animate_player(&player, &frame_speed);
   update_item_afterfx(item_fx);

   if (halftime_frame_speed == 0)
   {
      item_frame ^= 1;

      halftime_frame_speed = ANIMATION_SPEED * 2;
   }
   if (halftime_frame_speed % 3 == 0) { item_afterfx_frame^= 1; }

   if (frame_speed % 2 == 0)
   {
      if (player.hurt) player.draw ^= 1;
      if (!player.hurt) player.draw = true;
   }

   //Camera Look-ahead
   if (key[KEY_LEFT] && !key[KEY_RIGHT])
   {
      if (cam.look_ahead > -24) cam.look_ahead -=2;
   }
   if (key[KEY_RIGHT] && !key[KEY_LEFT])
   {
      if (cam.look_ahead < 24) cam.look_ahead += 2;
   }

   if (player.y >= (MAP_HEIGHT * TILE_SIZE) + 32)
   {
      player.health = 0;
   }
   
   check_cam();
}

/*
 * Stream function for opl Emulation
 */
void stream_opl()
{
   static int times_executed;

   opl_buffer = al_get_audio_stream_fragment(music_stream);
   
   if (!opl_buffer)
   {
      jlog("%d, opl_buffer returned null!", times_executed);
      return;
   }
   else
   {
      samples_count = adl_play(midi_player, BUFFER_SAMPLES *2, opl_buffer);
   }
   
   if (samples_count > 0)
   {
      al_set_audio_stream_fragment(music_stream, opl_buffer);
   }
   if (samples_count <= 0)
   {
      al_drain_audio_stream(music_stream);
   }

   times_executed++;
}

/*
 * Reset some things if they're out of camera range
 */
void reset_out_of_view_things()
{
   for (int i = 0; i < map->num_things; i++)
   {
      if (thing[i].type == ENEMY_BAD_ROBOT)
      {
         if (thing[i].x > cam.x + VIEWPORT_WIDTH || thing[i].y > cam.y + VIEWPORT_HEIGHT || thing[i].x + thing[i].width < cam.x || thing[i].y + thing[i].height < cam.y)
         {   
           thing[i].touched = false; //Reset greeting ability
         }
      }
   }
}

/*
 * Updates Orlo the Robot
 */
void update_orlo()
{
   const int fall_behind = 14;      //How many indecies behind is the read buffer? (Orlo has a delayed reaction when following Tally.)
   static int put_buffer_pos = 0;   //This is the index where we put the player x, y, direction, and animation frame, so Orlo can mimick it.
   static int read_buffer_pos = 0;  //This is the current index (set by fall_behind), of where Orlo reads from

   orlo_position_buffer[put_buffer_pos].x = player.x;
   orlo_position_buffer[put_buffer_pos].y = player.y;
   orlo_position_buffer[put_buffer_pos].direction = player.direction;
   orlo_position_buffer[put_buffer_pos].cur_frame = player.cur_frame;

   if (put_buffer_pos < ORLO_BUFFER_SIZE) put_buffer_pos++;

   if (put_buffer_pos >= ORLO_BUFFER_SIZE) 
   {
      put_buffer_pos = 0;
   }

   if (put_buffer_pos >= fall_behind) read_buffer_pos =  put_buffer_pos - fall_behind;    //Set the read buffer to be behind as many indecies as fall_behind indicates
   if (read_buffer_pos > put_buffer_pos) read_buffer_pos++;
   if (read_buffer_pos >= ORLO_BUFFER_SIZE) read_buffer_pos = 0;
   
   //Set Orlo's stuff
   orlo.x = orlo_position_buffer[read_buffer_pos].x;
   orlo.y = orlo_position_buffer[read_buffer_pos].y;
   orlo.direction = orlo_position_buffer[read_buffer_pos].direction;
   orlo.cur_frame = orlo_position_buffer[read_buffer_pos].cur_frame;

   //Check if Orlo picks up health
   //int x1, x2, x3;
   int tx, ty, ty2, ty3;
   t_map_pos *mp;
   t_map_pos *mp2;
   t_map_pos *mp3;
   tx = orlo.x + (orlo.direction ? 14 : 18);
   ty = (orlo.y + 1);
   ty2 = (orlo.y + 31);
   ty3 = (orlo.y + 16);
   
   mp = get_map_position(map, tx, ty);
   mp2 = get_map_position(map, tx, ty2);
   mp3 = get_map_position(map, tx, ty3);

   if (mp != NULL && mp2 != NULL && mp3 != NULL)
   {
      if (orlo.state == ORLO_STATE_NORMAL)
      {
         if (mp->item == ITEM_HEALTH && player.health == 8)
         {
            mp->item = 0;
            orlo.state = ORLO_STATE_W_HEALTH;
            activate_item_fx(mp, item_fx);
            play_sound(sounds.orlo_get_health, false);
            orlo_message_lifetime = 32;
            orlo_message_to_show = MSG_GET_HEALTH;
         }
         else if (mp2->item == ITEM_HEALTH && player.health == 8)
         {
            mp2->item = 0;
            orlo.state = ORLO_STATE_W_HEALTH;
            activate_item_fx(mp2, item_fx);
            play_sound(sounds.orlo_get_health, false);
            orlo_message_lifetime = 32;
            orlo_message_to_show = MSG_GET_HEALTH;
         }
         else if (mp3->item == ITEM_HEALTH && player.health == 8)
         {
            mp3->item = 0;
            orlo.state = ORLO_STATE_W_HEALTH;
            activate_item_fx(mp3, item_fx);
            play_sound(sounds.orlo_get_health, false);
            orlo_message_lifetime = 32;
            orlo_message_to_show = MSG_GET_HEALTH;
         } 
      }

   if (player.health < 8 && collision_check(&player, &orlo) && key[KEY_UP] && orlo.state == ORLO_STATE_W_HEALTH)
   {
      play_sound(sounds.orlo_give_health, false);
      orlo.state = ORLO_STATE_NORMAL;
      player.health = 8;
      orlo_gave_health = true;
      orlo_message_lifetime = 32;
      orlo_message_to_show = MSG_GIVE_HEALTH;
   }

   if (orlo_gave_health)
      {
         orlo_gave_health_life_span++;
      }
      if (orlo_gave_health_life_span == 32)
      {
         orlo_gave_health = false;
         orlo_gave_health_life_span = 0;
      }
   }
   if (orlo_message_lifetime > - 1) orlo_message_lifetime--;
   if (orlo_message_lifetime == -1) orlo_message_to_show = MSG_NONE;
 
}

void draw_orlo()
{
   if ( orlo.x > cam.x - orlo.width && 
      orlo.x < cam.x + VIEWPORT_WIDTH &&
      orlo.y > cam.y - orlo.height &&
      orlo.y < cam.y + VIEWPORT_HEIGHT)
   {
      if (orlo.direction == RIGHT)
            //al_draw_bitmap(orlo.frame[orlo.cur_frame], orlo.x - cam.x, orlo.y - cam.y, 0);
            al_draw_bitmap_region(orlo.bitmap, orlo.width * orlo.cur_frame, orlo.state * orlo.height, orlo.width, orlo.height, orlo.x - cam.x, orlo.y - cam.y, 0);
      if (orlo.direction == LEFT)
            //al_draw_bitmap(orlo.frame[orlo.cur_frame], orlo.x - cam.x, orlo.y - cam.y, ALLEGRO_FLIP_HORIZONTAL);
            al_draw_bitmap_region(orlo.bitmap, orlo.width * orlo.cur_frame, orlo.state * orlo.height, orlo.width, orlo.height, orlo.x - cam.x, orlo.y - cam.y, ALLEGRO_FLIP_HORIZONTAL);
   }
}

/*
 * The drawing function, called at redraw
 */
void update_screen()
{
   draw_map(view_port, graphics.tile_sheet, graphics.item_sheet, graphics.bg, &cam, map, &item_frame);
   draw_things(map, thing, &cam, map->num_things);
   if (!orlo_gave_health) draw_orlo();

   if (player.x + 32 > 0 && player.y + 32 > 0 && player.x < MAP_WIDTH * TILE_SIZE && player.y < MAP_HEIGHT * TILE_SIZE)   
      if (player.draw) draw_player(view_port, &cam, &player, player.direction);


   if (player.direction == RIGHT && player.muzzle_time) 
   {
      draw_laser();
      al_draw_bitmap(graphics.muzzle_flash, player.muzzle_x - cam.x, player.muzzle_y - cam.y, 0);
   }

   if (player.direction == LEFT && player.muzzle_time) 
   {
       draw_laser();
      al_draw_bitmap(graphics.muzzle_flash, player.muzzle_x - cam.x, player.muzzle_y - cam.y, ALLEGRO_FLIP_HORIZONTAL);
   }
   if (player_bullet.draw)
   {
      al_draw_bitmap_region(graphics.bullet_particle, (player.muzzle_time * 2) * 16, 0, 16, 16, player_bullet.end_x - cam.x - 8, player_bullet.end_y - cam.y - 8, 0);
      //al_draw_filled_circle(player_bullet.end_x - cam.x, player_bullet.end_y - cam.y, player.shoot_time /2, al_map_rgb(170, 0 ,0));
   }
   if (orlo_gave_health) draw_orlo();
   draw_item_fx(view_port, graphics.item_fx_sheet, &cam, item_fx, &item_afterfx_frame, &player);
   
   if (orlo_gave_health)
   {
      al_draw_bitmap_region(graphics.item_fx_sheet, 96, item_frame * 16, 16, 16, orlo.x + 8 - cam.x , orlo.y - orlo_gave_health_life_span - cam.y, 0);
   }
   if (orlo_message_lifetime > 0 && orlo_message_to_show == MSG_GET_HEALTH)  al_draw_textf(font_message, al_map_rgb(255,255,255), 1, 1, ALLEGRO_ALIGN_LEFT, "0R10: %s", ORLO_TXT_GET_HEALTH);
   if (orlo_message_lifetime > 0 && orlo_message_to_show == MSG_GIVE_HEALTH) al_draw_textf(font_message, al_map_rgb(255,255,255), 1, 1, ALLEGRO_ALIGN_LEFT, "0R10: %s", ORLO_TXT_GIVE_HEALTH);
   
   if (screen_flash > 0) al_clear_to_color(al_map_rgb(170,0,0));
   //Draw view_port to game, then draw game scaled to display.
   al_set_target_bitmap(game_bmp);
   
   al_draw_bitmap(view_port, VIEWPORT_X, VIEWPORT_Y, 0);
   if (game.demo_mode == PLAY)
   {
      if (item_frame == 0) al_draw_textf(font_status, al_map_rgb(255,255,255), 269, 136, ALLEGRO_ALIGN_CENTER, "DEMO");
      if (item_frame == 1) al_draw_textf(font_status, al_map_rgb(0,0,0), 269, 136, ALLEGRO_ALIGN_CENTER, "DEMO");
      //al_draw_textf(font, al_map_rgb(255,255,255), 0, 8, ALLEGRO_ALIGN_LEFT, "%d", demo_file_pos);
      
   }
   else if (game.demo_mode == RECORD)
   {
      if (item_frame == 0) al_draw_textf(font_status, al_map_rgb(255,255,255), 269, 136, ALLEGRO_ALIGN_CENTER, "RECORD");
      if (item_frame == 1) al_draw_textf(font_status, al_map_rgb(0,0,0), 269, 136, ALLEGRO_ALIGN_CENTER, "RECORD");
      //al_draw_textf(font, al_map_rgb(255,255,255), 0, 8, ALLEGRO_ALIGN_LEFT, "%d", demo_file_pos);
   }
   
   //al_draw_textf(font_status, al_map_rgb(255,255,255), 269, 102, ALLEGRO_ALIGN_CENTER, "%02d:%02d:%02d", minutes, seconds, centiseconds);
   al_draw_textf(font_status, al_map_rgb(255,255,255), 301, 18, ALLEGRO_ALIGN_RIGHT, "%09d", player.score);
   al_draw_textf(font_status, al_map_rgb(255,255,255), 18, 185, ALLEGRO_ALIGN_LEFT, map->name);
   al_draw_bitmap_region(graphics.health_bar, 0, player.health * 16, 64, 16, 238, 42, 0);
   al_set_target_backbuffer(display);
   
   al_draw_scaled_bitmap(game_bmp,
                         0,0,
                         screen.unscaled_w, screen.unscaled_h,
                         screen.x, screen.y,
                         screen.width, screen.height,
                         0);
   
   al_flip_display();
}

void update_enemies()
{
   for (int i = 0; i < map->num_things; i++)
   {
      if (thing[i].type == ENEMY_BAD_ROBOT)
      {
         if (thing[i].direction == RIGHT) thing[i].x += 1;
         if (thing[i].direction == LEFT) thing[i].x -= 1;
         if ( !is_ground(map, thing[i].x + 15, thing[i].y + 16) && thing[i].direction == RIGHT) thing[i].direction = LEFT;
         if ( !is_ground(map, thing[i].x + 1, thing[i].y + 16) && thing[i].direction == LEFT) thing[i].direction = RIGHT;
      }
   }
}

/*
 * Clean-ups for end of program
 */
void clean_up()
{
   if (game.level_needs_unloaded)
   {
      if(!unload_level())
      {
         jlog("Error unloading level!");
      }
      jlog("Level unloaded.");
   }

   al_destroy_audio_stream(music_stream);
   music_stream = NULL;
   if (music_stream == NULL)
   adl_close(midi_player);
   midi_player = NULL;
   if (midi_player == NULL)
   {
      jlog("midi_player destroyed.");
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
   bool redraw = false;

   char DEMO_FILENAME[32] = " ";
   FILE *demo_file = NULL;
   unsigned char key_buffer[640000] = { 0 };
   long filesize = 0;

   if (argc > 1)
   {
      for (int d = 1; d < argc; d++)
       {
         if (strcmp(argv[d], "-recorddemo") == 0)
         {
               if (argc -1 > d) d++;
               else 
               {
                  jlog("No file name for -recorddemo!");
                  return 1;
               }
               if (argv[d][0] != '-')
               {
                  strlcpy(DEMO_FILENAME, argv[d], strlen(argv[d]) +1);
                  game.demo_mode = RECORD;
               }   
               else
            {
               jlog("Must be file name after -recorddemo!");
               return 1;
            }
         }
         if (strcmp(argv[d], "-playdemo") == 0)
         { 
            if (argc -1 > d) d++;
            else 
            {
               jlog("No file name for -playdemo!");
               return 1;
            }
            if (argv[d][0] != '-')
            {
               strlcpy(DEMO_FILENAME, argv[d], strlen(argv[d]) + 1);
               FILE *fp = fopen(DEMO_FILENAME, "r");
               if (fp) fclose(fp);
               else
               {
                  jlog("Demo file doesn't exist!");
                  return 1;
               }
               game.demo_mode = PLAY;
            }
            else
            {
               jlog("Must be file name after -playdemo!");
               return 1;
            }
         }
      }
   }

   if (game.demo_mode == RECORD)
   {
      demo_file = fopen(DEMO_FILENAME, "wb");
   }

   if (game.demo_mode == PLAY)
   {
      demo_file = fopen(DEMO_FILENAME, "rb");
      fseek(demo_file, 0, SEEK_END);
      filesize = ftell(demo_file);
      rewind(demo_file);
      fread(key_buffer, filesize, 1, demo_file);
   }
   
   if (init_game(&display, &screen, &event_queue, &FPS_TIMER, FPS, &graphics, &font_status, &font_message, &midi_player, STREAM_FREQ, BUFFER_SAMPLES, &music_stream, &game, &game_bmp) == false)
   {
      jlog("Failed to init game!\n");
      return -1;
   }

   if (game.state == LOAD_LEVEL && !game.level_needs_unloaded)
   {
      if (!load_level(game.level, game.music))
      {
         jlog("Level failed to load!");
         return -1;
      }
      jlog("Level Loaded.");
   }

   al_start_timer(FPS_TIMER);
   jlog("FPS timer started.");

   int ticks = 0;
   while(game.state != QUIT)
   {
      ALLEGRO_EVENT ev;

      al_wait_for_event(event_queue, &ev);

      if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
      {
         check_key_down(&ev);
      }

      if (ev.type == ALLEGRO_EVENT_KEY_UP)
      {
         check_key_up(&ev);
      }

      if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
      {
         game.state = QUIT;
      }
      
      if (ev.type == ALLEGRO_EVENT_TIMER)
      {
         /* centiseconds += (100/FPS);
         if (centiseconds >= 100) 
         {
            centiseconds = 0;
            seconds++;
         }
         if (seconds == 60)
         {
            seconds = 0;
            minutes++;
         } */

         if (game.demo_mode == RECORD)
         {
            if (demo_file_pos < 640000)
            {
               key_buffer[demo_file_pos] = key[KEY_RIGHT];
               key_buffer[demo_file_pos + 1] = key[KEY_LEFT];
               key_buffer[demo_file_pos + 2] = key[KEY_JUMP];
               key_buffer[demo_file_pos + 3] = key[KEY_FIRE];
               demo_file_pos +=4 ;
            }
         }

         if (game.demo_mode == PLAY)
         {
            key[KEY_RIGHT] = key_buffer[demo_file_pos];
            key[KEY_LEFT] = key_buffer[demo_file_pos + 1];
            key[KEY_JUMP] = key_buffer[demo_file_pos + 2];
            key[KEY_FIRE] = key_buffer[demo_file_pos + 3];
            demo_file_pos += 4;
         }

         ++ticks;
         if (ticks == 1)
         {
            update_orlo();
            update_enemies();
            check_timer_logic();
         }
         
         reset_out_of_view_things();
         redraw = true;
      }
      
      if (ev.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
      {
         stream_opl();
      }

      if (redraw && al_is_event_queue_empty(event_queue))
      {
         
         redraw = false;
         update_screen();
         ticks = 0;
         
      }

   }
   clean_up();

   if (game.demo_mode == RECORD)
   {
      fwrite(key_buffer, sizeof(key_buffer), 1, demo_file);
   }

   if (game.demo_mode != NONE)
   {
      fclose(demo_file);
   }
   
   return 0;
}
