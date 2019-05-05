#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "tt_main.h"
#include "tt_map.h"
#include "tt_player.h"
#include "tt_collision.h"
#include "tt_items.h"

/* At some point I'll see if I can prune these globals,
   but they're staying for now. */
bool program_done = false;

const float FPS = 30;
int frame_speed = ANIMATION_SPEED;
int halftime_frame_speed = ANIMATION_SPEED * 2;

bool redraw = true;

const int DISPLAY_WIDTH = 320 * DISPLAY_MULTIPLIER;
const int DISPLAY_HEIGHT = 200 * DISPLAY_MULTIPLIER;

t_cam cam;
t_map *map = NULL;
t_player player = { .cur_frame = 0, .state = STOPPED, .vel_x = 4 };
t_item_afterfx *item_fx = NULL;

unsigned char item_frame = 0;
unsigned char item_afterfx_frame = 0;

bool key[12] = {false};

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *FPS_TIMER = NULL;
ALLEGRO_MOUSE_STATE mouse;

ALLEGRO_FONT *font = NULL;

//Bitmaps that get loaded from disk
ALLEGRO_BITMAP *tile_sheet = NULL;
ALLEGRO_BITMAP *item_sheet = NULL;
ALLEGRO_BITMAP *bg = NULL;
ALLEGRO_BITMAP *stat_border = NULL;
ALLEGRO_BITMAP *item_fx_sheet = NULL;
//ALLEGRO_BITMAP *player_start = NULL;

//Bitmaps that get drawn to
ALLEGRO_BITMAP *view_port = NULL;
ALLEGRO_BITMAP *console_map = NULL;
ALLEGRO_BITMAP *game = NULL;

//Sounds
float sfx_volume = 1.0;
ALLEGRO_SAMPLE *snd_pickup = NULL;
ALLEGRO_SAMPLE *snd_fall = NULL;
ALLEGRO_SAMPLE *snd_jump = NULL;
ALLEGRO_SAMPLE *snd_land = NULL;
ALLEGRO_SAMPLE *snd_hithead = NULL;

ALLEGRO_SAMPLE_ID *snd_jump_id = NULL;

//Music
float mus_volume = 0.5;
ALLEGRO_SAMPLE *music = NULL;
ALLEGRO_SAMPLE_INSTANCE *music_instance;


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

   //Audio
   if (!al_install_audio())
   {
      jlog("Failed to install audio!");
      return -1;
   }
   if (!al_init_acodec_addon())
   {
      jlog("Failed to initialize audio codec addon!");
      return -1;
   }
   jlog("Audio initialized.");

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

   stat_border = al_load_bitmap("data/border.png");
   if (stat_border == NULL)
   {
      jlog("Couldn't load border.png!");
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
   al_lock_bitmap(tile_sheet, al_get_bitmap_format(tile_sheet), ALLEGRO_LOCK_READONLY);

   item_sheet = al_load_bitmap("data/item_sheet.png");
   if (tile_sheet == NULL)
   {
      jlog("Couldn't load item_sheet.png!");
      return -1;
   }
   jlog("item_sheet.png loaded.");
   al_lock_bitmap(item_sheet, al_get_bitmap_format(tile_sheet), ALLEGRO_LOCK_READONLY);


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

   item_fx_sheet = al_load_bitmap("data/item_score.png");
   if (item_fx_sheet == NULL) { jlog("Couldn't load item_score.png"); return -1; }
   al_lock_bitmap(item_fx_sheet, al_get_bitmap_format(item_fx_sheet), ALLEGRO_LOCK_READONLY);

   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
   console_map = al_create_bitmap(64, 45);

   //Load sounds
   al_reserve_samples(1);
   snd_pickup = al_load_sample("data/sound/pickup.ogg");
   snd_fall = al_load_sample("data/sound/fall.ogg");
   snd_jump = al_load_sample("data/sound/jump.ogg");
   snd_land = al_load_sample("data/sound/land.ogg");
   snd_hithead = al_load_sample("data/sound/hithead.ogg");

   music = al_load_sample("data/music/music1.ogg");

   //Create the game bitmap that needs to be stretched to display
   game = al_create_bitmap(320, 200);

   //Load font
   font = al_load_bitmap_font("data/fonts/font.png");

   al_register_event_source(event_queue, al_get_display_event_source(display));
   al_register_event_source(event_queue, al_get_timer_event_source(FPS_TIMER));
   al_register_event_source(event_queue, al_get_keyboard_event_source());

   al_clear_to_color(al_map_rgb(0, 0, 0));

   al_flip_display();

   //Play music (Make this into a function at some point.
   music_instance = al_create_sample_instance(music);
   al_attach_sample_instance_to_mixer(music_instance, al_get_default_mixer());
   al_set_sample_instance_gain(music_instance, mus_volume);
   al_set_sample_instance_playmode(music_instance, ALLEGRO_PLAYMODE_LOOP);
   al_play_sample_instance(music_instance);

   jlog("Game initialized.");

   printf("%d", sizeof(t_map_pos));
   return 0;
}

/*************************************************
 * Plays a sound without all the boilerplate     *
 *************************************************/
void play_sound(ALLEGRO_SAMPLE *s)
{
   al_stop_samples();
   al_play_sample(s, sfx_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
}

/************************************************
 * The drawing function, called at redraw       *
 ************************************************/
void update_screen()
{

   draw_map(view_port, tile_sheet, item_sheet, bg, &cam, map, &item_frame);
   draw_player(view_port, &cam, &player, player.direction);
   draw_item_fx(view_port, item_fx_sheet, &cam, item_fx, &item_afterfx_frame, &player);
   show_player_hotspot(view_port, &cam, &player);
   draw_console_map(map, &player, console_map);

   //Draw view_port to game, then draw game scaled to display.
   al_set_target_bitmap(game);
   al_draw_bitmap(view_port, VIEWPORT_X, VIEWPORT_Y, 0);
   al_draw_bitmap(console_map, 238, 100, 0);
   al_draw_textf(font, al_map_rgb(255,255,255), 303, 18, ALLEGRO_ALIGN_RIGHT, "%010d", player.score);
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
      cam.x = player.x - ((VIEWPORT_WIDTH / 2) -16) + cam.look_ahead;
   }

   //Scroll when player is on ground or leaving view port.
   //On ground
   if ( player.on_ground && (cam.y < player.y - (VIEWPORT_HEIGHT / 2)) )
   {
      cam.y += 16;
   }
   if ( player.on_ground && (cam.y > player.y - (VIEWPORT_HEIGHT / 2) + 24) )
   {
      cam.y -= 16;
   }
   //Leaving view port
   if ( !player.on_ground && player.y < cam.y +8)
   {
      cam.y = player.y - 8;
   }
   if ( !player.on_ground && player.y + 32 > ((cam.y + (VIEWPORT_HEIGHT - 8))) )
   {
      cam.y = ((player.y + 32) - VIEWPORT_HEIGHT) + 8;
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
      case ALLEGRO_KEY_Z:
         key[KEY_Z] = true;
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
      case ALLEGRO_KEY_Z:
         key[KEY_Z] = false;
         break;
      case ALLEGRO_KEY_ESCAPE:
         program_done = true;
         break;
   }
}

/************************************************
 * Update the player and movements              *
 ************************************************/
void update_player()
{
   /* Special thanks to Johan Pietz, here. I borrowed
   from his code in Alex 4 a little. It's not copy
   and pasted, though! (Although, if you understand
   what it does, I guess it doesn't hurt every now
   and then.) */

   int old_x = player.x;
   int x1, x2, x3;
   int tx, ty, ty2, ty3;
   t_map_pos *mp;
   t_map_pos *mp2;
   t_map_pos *mp3;
   bool landed = false;
   bool jump_snd = false;

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
      #ifdef DEBUG
         player.x1 = x1;
         player.x2 = x2;
         player.x3 = x3;
      #endif // DEBUG

   }
   else
   {
      x1 = 14;
      x2 = 22;
      x3 = 18; //For detecting falling from edge.
      #ifdef DEBUG
         player.x1 = x1;
         player.x2 = x2;
         player.x3 = x3;
      #endif // DEBUG
   }

   /* Horizontal Tile Collision
      Basically checks three points in the player's
      Y position for each side of the player. If there's
      something there it sets the player's position back
      to its previous position before the collision occurred.
      But because we're not drawing here, it doesn't show the
      jerkiness of the process.*/
   if (is_ground(map, player.x + x1, player.y + 2 )) player.x = old_x; //top
   if (is_ground(map, player.x + x2, player.y + 2 )) player.x = old_x;

   if (is_ground(map, player.x + x1, player.y + 16)) player.x = old_x; //center
   if (is_ground(map, player.x + x2, player.y + 16)) player.x = old_x;

   if (is_ground(map, player.x + x1, player.y + 31)) player.x = old_x; //bottom
   if (is_ground(map, player.x + x2, player.y + 31)) player.x = old_x;

   /* Vertical movement
      Checks the two points below the player's feet
      if there's not a solid tile, it adds 4 to
      the Y velocity. Otherwise the player is standing
      on a solid tile. */
   if (!is_ground(map, player.x + x1, player.y + 32) && !is_ground(map, player.x +x2, player.y + 32))
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
      player.jump_pressed = false;
      player.vel_y = 0;

   }

   if (landed == true)
   {
      if (is_ground(map, player.x + x1, player.y + 32))
      {
         landed = false;
         play_sound(snd_land);
         #ifdef DEBUG
            printf("TALLY SMACKED THE GROUND!\n");
         #endif // DEBUG
      }
   }

   /* I added this in hopes that we could detect
   when a player falls off edge. Working so far.
   also detects if player barely lands on ledge
   and helps them out a little. */
   if(player.on_ground == true && !is_ground(map, player.x + x1, player.y + 32) && !is_ground(map, player.x +x3, player.y + 32) && !key[KEY_LCTRL])
   {
      //Play fall off ledge sound
      player.state = FALLING;
      if (player.direction == RIGHT) player.x += 4;
      if (player.direction == LEFT) player.x -= 4;
      player.vel_y += 24;
      play_sound(snd_fall);
      #ifdef DEBUG
         printf("OOPS!\n");
      #endif // DEBUG
   }
   else if(player.on_ground == true && !is_ground(map, player.x + x1, player.y + 32) && !is_ground(map, player.x +x3, player.y + 32) && key[KEY_LCTRL])
   {
      //Play fall off ledge sound
      if (player.direction == RIGHT) player.x += 4;
      if (player.direction == LEFT) player.x -= 4;
      //player.vel_y = -24;
      #ifdef DEBUG
         printf("FLY TALLY!\n");
      #endif // DEBUG
   }
   else if(player.on_ground == true && is_ground(map, player.x + x1, player.y + 32) && !is_ground(map, player.x +x3, player.y + 32))
   {
      if (player.direction == RIGHT) player.x += 4;
      if (player.direction == LEFT) player.x -= 4;
      #ifdef DEBUG
         printf("YOU MADE IT!\n");
      #endif // DEBUG
   }

   /* Jump button is pressed
      If the player is on the ground, and jump
      is pressed. The player jumps. Subtracting
      a number from the velocity, this gives the player
      a vertical upwards boost while the Y velocity is
      still being pulled in the opposite direction,
      simulating gravity. */
   if (key[KEY_LCTRL] && player.on_ground && !player.jump_pressed)
   {
      if (!is_ground(map, player.x + x1, player.y-1) && !is_ground(map, player.x + x2, player.y-1))
      {
         play_sound(snd_jump);
      }
      player.vel_y = -46;
      player.jump_pressed = true;
      player.on_ground = false;
   }
   if (!key[KEY_LCTRL])
   {
      //player.jump_pressed = false;
      if (player.vel_y < 0) player.vel_y /= 2;
   }

   #ifdef DEBUG
   if (key[KEY_Z])
   {
      player.vel_y = -8;
   }
   #endif // DEBUG

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
   while (is_ground(map, player.x + x1, player.y + 31))
   {
      player.y--;
      player.on_ground = true;
   }
   while (is_ground(map, player.x + x2, player.y + 31))
   {
      player.y--;
      player.on_ground = true;
   }

   /* Check Ceiling
      Same as above except at the players top. */
   while (is_ground(map, player.x + x1, player.y))
   {
      player.y++;
      player.vel_y = 0;
      if(!player.on_ground)
      {
         play_sound(snd_hithead);
      }

   }
   while (is_ground(map, player.x + x2, player.y))
   {
      player.y++;
      player.vel_y = 0;
      if(!player.on_ground)
      {
         play_sound(snd_hithead);
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
         if (mp2->item == ITEM_HEALTH && player.health < 8) { player.health = 8; mp2->item = 0; activate_item_fx(mp2, item_fx); jlog("Health: %d", player.health); play_sound(snd_pickup); player.score += 100;}
         if (mp->item == ITEM_HEALTH && player.health < 8) { player.health = 8; mp->item = 0;  activate_item_fx(mp, item_fx); jlog("Health: %d", player.health); play_sound(snd_pickup); player.score += 100;}
         if (mp3->item == ITEM_HEALTH && player.health < 8) { player.health = 8;mp3->item = 0; activate_item_fx(mp3, item_fx); jlog("Health: %d", player.health); play_sound(snd_pickup); player.score += 100;}

         // Not-Health
         if (mp->item != ITEM_HEALTH && mp2->item != ITEM_HEALTH && mp3->item != ITEM_HEALTH)
         {
            play_sound(snd_pickup);
            jlog("Score: %d", player.score);
         }
      }
   }
   //printf("player.state: %d\n", player.state);
   //printf("player.vel_y: %d\n", player.vel_y);
   //printf("player.x: %d\n", player.x);
}

/************************************************
 * Checks logic that needs to be timed by FPS   *
 ************************************************/
void check_timer_logic(ALLEGRO_EVENT *ev)
{
   //Frames
   if (ev->timer.source == FPS_TIMER)
   {


      frame_speed--; //This eliminates the need for an animation timer.
      halftime_frame_speed--; //This is for 2 frame animations like for blinking items

      update_player();
      animate_player(&player, &frame_speed);
      update_item_afterfx(item_fx);

      if (halftime_frame_speed == 0) { item_frame ^= 1; halftime_frame_speed = ANIMATION_SPEED * 2; }
      if (halftime_frame_speed % 3 == 0) { item_afterfx_frame^= 1; }


      //Camera Look-ahead
      if (key[KEY_LEFT] && !key[KEY_RIGHT])
      {
         if (cam.look_ahead > -24) cam.look_ahead -=1;
      }
      if (key[KEY_RIGHT] && !key[KEY_LEFT])
      {
         if (cam.look_ahead < 24) cam.look_ahead += 1;
      }
   }

   check_cam();
}

/************************************************
 * Clean-ups for end of program                 *
 ************************************************/
void clean_up()
{
   destroy_map(map);
   map = NULL;

   destroy_item_afterfx(item_fx);
   item_fx = NULL;

   al_unlock_bitmap(tile_sheet);
   al_unlock_bitmap(item_sheet);
   al_unlock_bitmap(bg);
   al_unlock_bitmap(stat_border);
   al_unlock_bitmap(player.bitmap);
   al_unlock_bitmap(item_fx_sheet);
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

   item_fx = create_item_after_fx(map);

   al_set_target_bitmap(game);
   al_draw_bitmap(stat_border, 0, 0, 0);

   //This is the main loop for now

   al_start_timer(FPS_TIMER);
   jlog("FPS timer started.");

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

