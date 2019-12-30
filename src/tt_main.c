#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <adlmidi.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include "tt_main.h"
#include "tt_thing.h"
#include "tt_map.h"
#include "tt_player.h"
#include "tt_collision.h"
#include "tt_items.h"
#include "tt_bullet.h"


char DEMO_FILENAME[32] = " ";
int frames = 0;
int minutes = 0;
int seconds = 0;
int centiseconds = 0;

#define BUFFER_SAMPLES 2048
#define STREAM_FREQ 44100
/* At some point I'll see if I can prune these globals,
   but they're staying for now. */
int demo_file_pos = 0;

FILE *demo_file = NULL;
bool program_done = false;
t_game game = { .state = LOAD_LEVEL, .next_state = LOAD_LEVEL };

const float FPS = 30;
int frame_speed = ANIMATION_SPEED;
int halftime_frame_speed = ANIMATION_SPEED * 2;

bool redraw = true;

t_bullet player_bullet;

t_screen screen = { .unscaled_w = 320, .unscaled_h = 200 };

t_cam cam;
t_map *map = NULL;
t_player player = { .cur_frame = 0, .state = STOPPED, .vel_x = 4 };
t_item_afterfx *item_fx = NULL;

t_thing thing[MAX_THINGS];

unsigned char item_frame = 0;
unsigned char item_afterfx_frame = 0;

bool key[16] = { false };

#define ORLO_BUFFER_SIZE 32 

typedef struct
{
   int x;
   int y;
} t_orlo;

t_orlo orlo_position_buffer[ORLO_BUFFER_SIZE] = { 0 };

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *FPS_TIMER = NULL;
ALLEGRO_MOUSE_STATE mouse;

ALLEGRO_FONT *font = NULL;
ALLEGRO_FONT *fps_font = NULL;

//Bitmaps that get loaded from disk
ALLEGRO_BITMAP *loading = NULL;
ALLEGRO_BITMAP *border = NULL;
ALLEGRO_BITMAP *tile_sheet = NULL;
ALLEGRO_BITMAP *item_sheet = NULL;
ALLEGRO_BITMAP *bg = NULL;
ALLEGRO_BITMAP *stat_border = NULL;
ALLEGRO_BITMAP *item_fx_sheet = NULL;
ALLEGRO_BITMAP *health_bar = NULL;
ALLEGRO_BITMAP *bullet_blue = NULL;
ALLEGRO_BITMAP *muzzle_flash = NULL;
ALLEGRO_BITMAP *bullet_particle = NULL;
ALLEGRO_BITMAP *laser = NULL;

//Bitmaps that get drawn tom
ALLEGRO_BITMAP *view_port = NULL;
ALLEGRO_BITMAP *game_bmp = NULL;

//Sounds
float sfx_volume = 1.0;
ALLEGRO_SAMPLE *snd_pickup = NULL;
ALLEGRO_SAMPLE *snd_hurt = NULL;
ALLEGRO_SAMPLE *snd_health = NULL;
ALLEGRO_SAMPLE *snd_fall = NULL;
ALLEGRO_SAMPLE *snd_jump = NULL;
ALLEGRO_SAMPLE *snd_land = NULL;
ALLEGRO_SAMPLE *snd_hithead = NULL;
ALLEGRO_SAMPLE *snd_shoot = NULL;
ALLEGRO_SAMPLE *snd_friend = NULL;

ALLEGRO_SAMPLE_ID *snd_jump_id = NULL;

ALLEGRO_AUDIO_STREAM *music_stream = NULL;
struct ADL_MIDIPlayer *midi_player = NULL;

short *opl_buffer = NULL;
int samples_count = 0;


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

   if (!al_init_image_addon())
   {
      jlog("Failed to initialize image_addon!");
      return -1;
   }
   jlog("Image add-on initialized.");
   //al_set_new_display_adapter(1);
   screen.width = 960;
   screen.height = 600;
   //al_set_new_display_flags(ALLEGRO_NOFRAME);
   al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
   display = al_create_display(screen.width, screen.height);
   if(!display)
   {
      jlog("Failed to create display!");
      return -1;
   }
   //Special thanks to dthompson from the allegro.cc forums for the setup below
   screen.factor_x = al_get_display_width(display) / screen.unscaled_w;
   screen.factor_y = al_get_display_height(display) / screen.unscaled_h;
   screen.factor = (screen.factor_y < screen.factor_x) ? screen.factor_y : screen.factor_x;
   screen.width = screen.unscaled_w * screen.factor;
   screen.height = screen.unscaled_h * screen.factor;
   screen.x = (al_get_display_width(display) / 2) - (screen.width/2);
   screen.y = (al_get_display_height(display) / 2) - (screen.height/2);
   jlog("Display Created.");
   
   // Load/Draw the fill-in border
   border = al_load_bitmap("data/bg_border.png");
   if (border == NULL)
   {
      jlog("Couldn't load bg_border.png!");
      return -1;
   }
   for (int y = 0; y < (al_get_display_height(display) / (32 * screen.factor)) + 1; y++)
   {
      for (int x = 0; x < (al_get_display_width(display) / (32 * screen.factor)) + 1; x++)
      {
         al_draw_scaled_bitmap(border, 0, 0, 32, 32, x * (32 * screen.factor), y * (32 * screen.factor), 32 * screen.factor, 32 * screen.factor, 0);
      }
   }
   //Load/Draw Loading bitmap and flip the display
   loading = al_load_bitmap("data/loading.png");
   if (loading == NULL)
   {
      jlog("Couldn't load loading.png!");
   }
   else
   {
      al_draw_scaled_bitmap(loading, 0, 0, screen.unscaled_w, screen.unscaled_h, screen.x, screen.y, screen.width, screen.height, 0);
      al_flip_display();
   }

   event_queue = al_create_event_queue();
   if (!event_queue)
   {
      jlog("Failed to create event queue!");
      return -1;
   }
   jlog("Event queue created.");

   FPS_TIMER = al_create_timer(ALLEGRO_BPS_TO_SECS(FPS));
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
   //Font Add-ons
   if(!al_init_ttf_addon())
   {
      jlog("Failed to install ttf addon!");
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

   #ifdef DEBUG
   al_set_window_title(display, "Tally Trauma -- DEBUG");
   #endif // DEBUG

   #ifdef RELEASE
   al_set_window_title(display, "Tally Trauma");
   #endif // RELEASE

   //Load font
   font = al_load_bitmap_font("data/fonts/font.png");
   fps_font = al_load_font("data/hack.ttf", 12, 0);

   //ADLMIDI
   midi_player = adl_init(STREAM_FREQ);
   //adl_setRunAtPcmRate(midi_player, 1);
   //adl_setTempo(midi_player, 1.0);
   adl_setLoopEnabled(midi_player, 1);
   adl_switchEmulator(midi_player, ADLMIDI_EMU_DOSBOX);
   adl_openBankFile(midi_player, "data/gm.wopl");
   if (adl_openFile(midi_player, "data/music/opl2.mid") < 0)
   {
      jlog("Couldn't open music file: %s", adl_errorInfo(midi_player));
      adl_close(midi_player);
      return 1;
   }

   music_stream = al_create_audio_stream(2, BUFFER_SAMPLES, STREAM_FREQ, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
   
   al_register_event_source(event_queue, al_get_display_event_source(display));
   al_register_event_source(event_queue, al_get_timer_event_source(FPS_TIMER));
   al_register_event_source(event_queue, al_get_keyboard_event_source());
   al_register_event_source(event_queue, al_get_audio_stream_event_source(music_stream));

   //Create the game bitmap that needs to be stretched to display
   game_bmp = al_create_bitmap(320, 200);

   al_reserve_samples(8);

   jlog("Game initialized.");
   jlog("Screen size factor: %d", screen.factor);

   game.level = LEVEL_1;
   /* game.music = MUSIC_1; */
   al_set_mixer_gain(al_get_default_mixer(), 1.0f); //Turn down the volume during development
   al_set_audio_stream_gain(music_stream, 1.0f);
   al_attach_audio_stream_to_mixer(music_stream, al_get_default_mixer());
   al_set_audio_stream_playing(music_stream, false);
   jlog("DEPTH: %d\n", al_get_mixer_depth(al_get_default_mixer()));
   return 0;
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

/*************************************************
 * Loads a level up at LOAD_LEVEL game state     *
 *************************************************/
 bool load_level(char *map_file, char *music_file)
 {
   //Display loading image
   al_draw_scaled_bitmap(loading, 0, 0, screen.unscaled_w, screen.unscaled_h, screen.x, screen.y, screen.width, screen.height, 0);
   al_flip_display();

   //Load map
   map = load_map(map_file); //Create an empty map
   if (map == NULL)
   {
      jlog("In file %s, Line %d. Couldn't create an empty map!" __FILE__, __LINE__);
      return false;
   }
   jlog("Map loaded.");

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
   stat_border = al_load_bitmap("data/stat_border.png");
   if (stat_border == NULL)
   {
      jlog("Couldn't load stat_border.png!");
      return false;
   }

   bg = al_load_bitmap("data/bg_1.png");
   if (bg == NULL)
   {
      jlog("Couldn't load bg1.png!");
      return false;
   }

   tile_sheet = al_load_bitmap("data/tile_sheet.png");
   if (tile_sheet == NULL)
   {
      jlog("Couldn't load tile_sheet.png!");
      return false;
   }

   item_sheet = al_load_bitmap("data/item_sheet.png");
   if (tile_sheet == NULL)
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
      jlog("Player frame %d created and locked.", j);
   }

   item_fx_sheet = al_load_bitmap("data/item_score.png");
   if (item_fx_sheet == NULL) { jlog("Couldn't load item_score.png"); return false;; }
  
   health_bar = al_load_bitmap("data/health_bar.png");
   if (health_bar == NULL) { jlog("Couldn't load health_bar.png"); return false; }

   muzzle_flash = al_load_bitmap("data/muzzle_flash.png");
   if (muzzle_flash == NULL) { jlog("Couldn't load muzzle_flash.png"); return false; }
   
   bullet_particle = al_load_bitmap("data/particle.png");
   if (bullet_particle == NULL) { jlog("Couldn't load particle.png"); return false; }

   laser = al_load_bitmap("data/laser.png");
   if (laser == NULL) { jlog("Couldn't load laser.png"); return false; }



   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

   //Load sounds
   snd_fall = al_load_sample("data/sound/fall.wav");
   snd_jump = al_load_sample("data/sound/jump.wav");
   snd_land = al_load_sample("data/sound/land.wav");
   snd_hithead = al_load_sample("data/sound/hithead.wav");
   snd_pickup = al_load_sample("data/sound/pickup.wav");
   snd_health = al_load_sample("data/sound/health.wav");
   snd_hurt = al_load_sample("data/sound/hurt.wav");
   snd_shoot = al_load_sample("data/sound/shoot.wav");
   snd_friend = al_load_sample("data/sound/friend.wav");

   al_set_audio_stream_playing(music_stream, true);

   //Load things
   load_things(thing, map);

   //draw the status border
   al_set_target_bitmap(game_bmp);
   al_draw_bitmap(stat_border, 0, 0, 0);

   game.level_needs_unloaded = true;

   centiseconds = 0;
   seconds = 0;
   minutes = 0;
   return true; //Returns true on success
 }

/*************************************************
 * Unloads a level                               *
 *************************************************/
bool unload_level()
{
   destroy_map(map);
   map = NULL;
   if (map == NULL) jlog("map unloaded.");

   destroy_item_afterfx(item_fx);
   item_fx = NULL;
   if (item_fx == NULL) jlog("item_fx unloaded.");

   al_destroy_bitmap(tile_sheet);
   tile_sheet = NULL;
   if (tile_sheet == NULL) jlog("tile_sheet unloaded.");
   al_destroy_bitmap(item_sheet);
   item_sheet = NULL;
   if (item_sheet == NULL) jlog("item_sheet unloaded.");

   al_destroy_bitmap(bg);
   bg = NULL;
   if (bg == NULL) jlog("bg unloaded.");

   al_destroy_bitmap(stat_border);
   stat_border = NULL;
   if (stat_border == NULL) jlog("stat_border unloaded.");

   al_destroy_bitmap(item_fx_sheet);
   item_fx_sheet = NULL;
   if (item_fx_sheet == NULL) jlog("item_fx_sheet unloaded.");

   al_destroy_bitmap(health_bar);
   health_bar = NULL;
   if (health_bar == NULL) jlog("health_bar unloaded.");

   al_destroy_bitmap(muzzle_flash);
   muzzle_flash = NULL;
   if (muzzle_flash == NULL) jlog("muzzle_flash unloaded.");

   al_destroy_bitmap(bullet_particle);
   bullet_particle = NULL;
   if (bullet_particle == NULL) jlog("bullet_particle unloaded.");

   al_destroy_bitmap(laser);
   laser = NULL;
   if (laser == NULL) jlog("laser unloaded.");

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

   al_destroy_sample(snd_jump);
   snd_jump = NULL;

   al_destroy_sample(snd_land);
   snd_land = NULL;

   al_destroy_sample(snd_hithead);
   snd_hithead = NULL;

   al_destroy_sample(snd_pickup);
   snd_pickup = NULL;

   al_destroy_sample(snd_health);
   snd_health = NULL;

   al_destroy_sample(snd_hurt);
   snd_hurt = NULL;

   al_destroy_sample(snd_shoot);
   snd_shoot = NULL;

   game.level_needs_unloaded = false;
   return true; //Returns true on success
}


/*************************************************
 * Plays a sound without all the boilerplate     *
 *************************************************/
void play_sound(ALLEGRO_SAMPLE *s, bool interupt)
{
   if (interupt) al_stop_samples();
   al_play_sample(s, sfx_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
}

/************************************************
 * Draw laser                                   *
 ************************************************/
void draw_laser()
{
   if (player.direction == RIGHT)
   {
      for (int x = player_bullet.start_x; x < player_bullet.end_x; x++)
      {
         al_draw_bitmap(laser, x - cam.x , player.muzzle_y - cam.y + 6, 0);
      }
   }

   if (player.direction == LEFT)
   {
      for (int x = player_bullet.start_x; x > player_bullet.end_x; x--)
      {
          al_draw_bitmap(laser, x - cam.x , player.muzzle_y - cam.y + 6, 0);
      }
   }
}

/************************************************
 * The drawing function, called at redraw       *
 ************************************************/
void update_screen()
{
   draw_map(view_port, tile_sheet, item_sheet, bg, &cam, map, &item_frame);
   draw_things(map, thing, &cam, map->num_things);

   if (player.x + 32 > 0 && player.y + 32 > 0 && player.x < MAP_WIDTH * TILE_SIZE && player.y < MAP_HEIGHT * TILE_SIZE)   
      if (player.draw) draw_player(view_port, &cam, &player, player.direction);


   if (player.direction == RIGHT && player.muzzle_time) 
   {
      draw_laser();
      al_draw_bitmap(muzzle_flash, player.muzzle_x - cam.x, player.muzzle_y - cam.y, 0);
   }

   if (player.direction == LEFT && player.muzzle_time) 
   {
       draw_laser();
      al_draw_bitmap(muzzle_flash, player.muzzle_x - cam.x, player.muzzle_y - cam.y, ALLEGRO_FLIP_HORIZONTAL);
   }
   if (player_bullet.draw)
   {
      al_draw_bitmap_region(bullet_particle, (player.muzzle_time * 2) * 16, 0, 16, 16, player_bullet.end_x - cam.x - 8, player_bullet.end_y - cam.y - 8, 0);
      //al_draw_filled_circle(player_bullet.end_x - cam.x, player_bullet.end_y - cam.y, player.shoot_time /2, al_map_rgb(170, 0 ,0));
   }
   
   draw_item_fx(view_port, item_fx_sheet, &cam, item_fx, &item_afterfx_frame, &player);
   
   //Draw view_port to game, then draw game scaled to display.
   al_set_target_bitmap(game_bmp);
    
   al_draw_bitmap(view_port, VIEWPORT_X, VIEWPORT_Y, 0);
   if (game.demo_mode == PLAY)
   {
      if (item_frame == 0) al_draw_textf(font, al_map_rgb(255,255,255), 269, 136, ALLEGRO_ALIGN_CENTER, "DEMO");
      if (item_frame == 1) al_draw_textf(font, al_map_rgb(0,0,0), 269, 136, ALLEGRO_ALIGN_CENTER, "DEMO");
      //al_draw_textf(font, al_map_rgb(255,255,255), 0, 8, ALLEGRO_ALIGN_LEFT, "%d", demo_file_pos);
      
   }
   else if (game.demo_mode == RECORD)
   {
      if (item_frame == 0) al_draw_textf(font, al_map_rgb(255,255,255), 269, 136, ALLEGRO_ALIGN_CENTER, "RECORD");
      if (item_frame == 1) al_draw_textf(font, al_map_rgb(0,0,0), 269, 136, ALLEGRO_ALIGN_CENTER, "RECORD");
      //al_draw_textf(font, al_map_rgb(255,255,255), 0, 8, ALLEGRO_ALIGN_LEFT, "%d", demo_file_pos);
   }
   
   al_draw_textf(font, al_map_rgb(255,255,255), 269, 102, ALLEGRO_ALIGN_CENTER, "%02d:%02d:%02d", minutes, seconds, centiseconds);
   al_draw_textf(font, al_map_rgb(255,255,255), 301, 18, ALLEGRO_ALIGN_RIGHT, "%09d", player.score);
   al_draw_textf(font, al_map_rgb(255,255,255), 18, 185, ALLEGRO_ALIGN_LEFT, map->name);
   al_draw_bitmap_region(health_bar, 0, player.health * 16, 64, 16, 238, 42, 0);
   al_set_target_backbuffer(display);
   
   al_draw_scaled_bitmap(game_bmp,
                         0,0,
                         screen.unscaled_w, screen.unscaled_h,
                         screen.x, screen.y,
                         screen.width, screen.height,
                         0);
   
   al_flip_display();
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
      case ALLEGRO_KEY_LSHIFT:
         key[KEY_LSHIFT] = true;
         break;
      case ALLEGRO_KEY_X:
         key[KEY_X] = true;
         break;
      case ALLEGRO_KEY_T:
         key[KEY_T] = true;
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
      case ALLEGRO_KEY_LSHIFT:
         key[KEY_LSHIFT] = false;
         break;
      case ALLEGRO_KEY_X:
         key[KEY_X] = false;
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
         for (int y = 0; y < (al_get_display_height(display) / (32 * screen.factor)) + 1; y++)
         {
            for (int x = 0; x < (al_get_display_width(display) / (32 * screen.factor)) + 1; x++)
            {
               al_draw_scaled_bitmap(border, 0, 0, 32, 32, x * (32 * screen.factor), y * (32 * screen.factor), 32 * screen.factor, 32 * screen.factor, 0);
            }
         }
         jlog("Display Created.");

         break;


      case ALLEGRO_KEY_P:
         player.x = map->player_start_x;
         player.y = map->player_start_y;
         cam.x = player.x - VIEWPORT_WIDTH / 2 + 24;
         cam.y = player.y - VIEWPORT_HEIGHT / 2 + 16;
         check_cam();
         break;

      case ALLEGRO_KEY_T:
         key[KEY_T] = false;
         break;

      case ALLEGRO_KEY_ESCAPE:
         game.state = QUIT;
         break;
   }
}

/************************************************
 * Check bullet collisions                      *
 ************************************************/
bool check_bullet_collision()
{

   if (player.direction == RIGHT)
   {
      player_bullet.start_x = player.x + 25;
      player_bullet.start_y = player.y + 18;
      for (int x = player_bullet.start_x; x < cam.x + VIEWPORT_WIDTH + 16; x++)
      {
         if (is_ground(map, x, player_bullet.start_y, &player))
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
         if (is_ground(map, x, player_bullet.start_y, &player))
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


/************************************************
 * Update the player and movements              *
 ************************************************/
void update_player()
{
   /* NOTE: Special thanks to Johan Pietz, here. This part is
   very similar to the player code in Alex the Allegator 4. */

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
      jerkiness of the process.*/
   if (player.y + 31 < MAP_HEIGHT * TILE_SIZE)
   {
      if (player.y + 32 > 0)
      {
         if (is_ground(map, player.x + x1, player.y + 2, &player )) player.x = old_x; //top
         if (is_ground(map, player.x + x2, player.y + 2, &player )) player.x = old_x;

         if (is_ground(map, player.x + x1, player.y + 16, &player )) player.x = old_x; //center
         if (is_ground(map, player.x + x2, player.y + 16, &player )) player.x = old_x;

         if (is_ground(map, player.x + x1, player.y + 31, &player )) player.x = old_x; //bottom
         if (is_ground(map, player.x + x2, player.y + 31, &player )) player.x = old_x;
      }
   }
   /* Vertical movement
      Checks the two points below the player's feet
      if there's not a solid tile, it adds 4 to
      the Y velocity. Otherwise the player is standing
      on a solid tile. */
   if ( (!is_ground(map, player.x + x1, player.y + 32, &player) && !is_ground(map, player.x +x2, player.y + 32, &player)) )
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
      if (is_ground(map, player.x + x1, player.y + 32, &player))
      {
         landed = false;
         play_sound(snd_land, false);
      }
   }

   /* I added this in hopes that we could detect
   when a player falls off edge. Working so far.
   also detects if player barely lands on ledge
   and helps them out a little. */
   if(player.on_ground == true && !is_ground(map, player.x + x1, player.y + 32, &player) && !is_ground(map, player.x +x3, player.y + 32, &player) && !key[KEY_Z] && player.y + 31 > 0)
   {
      //Play fall off ledge sound
      player.state = FALLING;
      if (player.direction == RIGHT) player.x += 4;
      if (player.direction == LEFT) player.x -= 4;
      player.vel_y += 24;
      play_sound(snd_fall, false);
   }
   else if(player.on_ground == true && !is_ground(map, player.x + x1, player.y + 32, &player) && !is_ground(map, player.x +x3, player.y + 32, &player) && key[KEY_Z] && player.y + 31 > 0)
   {
      //Play fall off ledge sound
      if (player.direction == RIGHT) player.x += 4;
      if (player.direction == LEFT) player.x -= 4;
      //player.vel_y = -24;
   }
   else if(player.on_ground == true && is_ground(map, player.x + x1, player.y + 32, &player) && !is_ground(map, player.x +x3, player.y + 32, &player) && player.y + 31 > 0)
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
      if (key[KEY_Z] && player.on_ground && !player.jump_pressed)
      {
         if (player.y + 1 > 0 && !is_ground(map, player.x + x1, player.y-1, &player) && !is_ground(map, player.x + x2, player.y-1, &player))
         {
            if (!is_ground(map, player.x + x1, player.y-1, &player) && !is_ground(map, player.x + x2, player.y-1, &player) && player.y + 31 > 0)
            {
               play_sound(snd_jump, false);
            }
            player.vel_y = -46;
            player.jump_pressed = true;
            player.on_ground = false;
         }
         else if (player.y + 1 < 0 && is_ground(map, player.x + x1, player.y-1, &player) && is_ground(map, player.x + x2, player.y-1, &player))
         {
            if (is_ground(map, player.x + x1, player.y-1, &player) && is_ground(map, player.x + x2, player.y-1, &player) && player.y + 31 > 0)
            {
               play_sound(snd_jump, false);
            }
            player.vel_y = -46;
            player.jump_pressed = true;
            player.on_ground = false;
         }
      }

   if (!key[KEY_Z])
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
      while (is_ground(map, player.x + x1, player.y + 31, &player) && player.y + 31 > 0)
      {
         player.y--;
         player.on_ground = true;
      }
      while (is_ground(map, player.x + x2, player.y + 31, &player) && player.y + 31 > 0)
      {
         player.y--;
         player.on_ground = true;
      }
   }

   /* Check Ceiling
      Same as above except at the players top. */
   if (player.y > 0)
   {
      while (is_ground(map, player.x + x1, player.y, &player))
      {
         player.y++;
         player.vel_y = 0;
         if(player.state == JUMPING)
         {
            play_sound(snd_hithead, false);
         }
         player.state = FALLING;

      }
      while (is_ground(map, player.x + x2, player.y, &player))
      {
         player.y++;
         player.vel_y = 0;
         if(player.state == JUMPING)
         {
            play_sound(snd_hithead, false);
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
            play_sound(snd_health, false);
            player.score += 100;
         }
         else if (mp2->item == ITEM_HEALTH && player.health < 8)
         {
            player.health = 8;
            mp2->item = 0;
            activate_item_fx(mp2, item_fx);
            jlog("Health: %d", player.health);
            play_sound(snd_health, false);
            player.score += 100;
         }
         else if (mp3->item == ITEM_HEALTH && player.health < 8)
         {
            player.health = 8;
            mp3->item = 0;
            activate_item_fx(mp3, item_fx);
            jlog("Health: %d", player.health);
            play_sound(snd_health, false);
            player.score += 100;
         }
         // Not-Health sound
         else if (mp->item != ITEM_HEALTH && mp2->item != ITEM_HEALTH && mp3->item != ITEM_HEALTH)
         {
            play_sound(snd_pickup, false);
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
            play_sound(snd_hurt, false);
            player.hurt = PLAYER_HURT_TIME;
            if(player.health) player.health--;
         }
         if (thing[i].type == THING_ORLO)
         {
            if (thing[i].touched == false) play_sound(snd_friend, false);
            thing[i].touched = true;
         }
      }
   }
   if (player.hurt)
   {
      player.hurt--;
   }

   //Shooting time
   if (key[KEY_X] && !player.shoot_time)
   {
      play_sound(snd_shoot, false);
      player.shoot_time = 5;
      player.muzzle_time = 3;
   }
   if (player.muzzle_time) player.muzzle_time--;
   else player_bullet.draw = false;

   if (player.shoot_time == 5) check_bullet_collision();
   if (player.shoot_time) player.shoot_time--;
}

/************************************************
 * Checks logic that needs to be timed by FPS   *
 ************************************************/
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
      if (cam.look_ahead > -24) cam.look_ahead -=1;
   }
   if (key[KEY_RIGHT] && !key[KEY_LEFT])
   {
      if (cam.look_ahead < 24) cam.look_ahead += 1;
   }

   if (player.y >= (MAP_HEIGHT * TILE_SIZE) + 32)
   {
      player.health = 0;
   }
   
   check_cam();
}

/************************************************
 * Stream function for opl Emulation            *
 ************************************************/
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

/****************************************************
 * Reset some things if they're out of camera range *
 ****************************************************/
void reset_out_of_view_things()
{
   for (int i = 0; i < map->num_things; i++)
   {
      if (thing[i].type == THING_ORLO)
      {
         if (thing[i].x > cam.x + VIEWPORT_WIDTH || thing[i].y > cam.y + VIEWPORT_HEIGHT || thing[i].x + thing[i].width < cam.x || thing[i].y + thing[i].height < cam.y)
         {   
           thing[i].touched = false; //Reset greeting ability
         }
      }
   }
}

void update_orlo()
{
   static int orlo_buffer_pos = 8; 
   static int orlo_pos_delay = 0;
   orlo_position_buffer[orlo_buffer_pos].x = player.x;
   orlo_position_buffer[orlo_buffer_pos].y = player.y;
   if (orlo_buffer_pos < ORLO_BUFFER_SIZE) orlo_buffer_pos ++;
   if (orlo_pos_delay < ORLO_BUFFER_SIZE) orlo_pos_delay++;
   if (orlo_buffer_pos >= ORLO_BUFFER_SIZE) orlo_buffer_pos = 0;
   if (orlo_pos_delay >= ORLO_BUFFER_SIZE) orlo_pos_delay = 0;
   //I may make Orlo his own entity instead of placing him as a thing. But for now this is how it is.
   for (int i = 0; i < map->num_things; i++)
   {
      if (thing[i].type == THING_ORLO)
      {
         thing[i].x = orlo_position_buffer[orlo_pos_delay].x;
         thing[i].y = orlo_position_buffer[orlo_pos_delay].y;
      }
   }
}

/************************************************
 * Clean-ups for end of program                 *
 ************************************************/
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
   
   if (init_game() != 0)
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
         centiseconds += (100/FPS);
         if (centiseconds >= 100) 
         {
            centiseconds = 0;
            seconds++;
         }
         if (seconds == 60)
         {
            seconds = 0;
            minutes++;
         }

         if (game.demo_mode == RECORD)
         {
            if (demo_file_pos < 640000)
            {
               key_buffer[demo_file_pos] = key[KEY_RIGHT];
               key_buffer[demo_file_pos + 1] = key[KEY_LEFT];
               key_buffer[demo_file_pos + 2] = key[KEY_Z];
               key_buffer[demo_file_pos + 3] = key[KEY_X];
               demo_file_pos +=4 ;
            }
         }

         if (game.demo_mode == PLAY)
         {
            key[KEY_RIGHT] = key_buffer[demo_file_pos];
            key[KEY_LEFT] = key_buffer[demo_file_pos + 1];
            key[KEY_Z] = key_buffer[demo_file_pos + 2];
            key[KEY_X] = key_buffer[demo_file_pos + 3];
            demo_file_pos += 4;
         }

         ++ticks;
         if (ticks == 1)
         {
            update_orlo();
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
