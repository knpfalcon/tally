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
#include "tt_map.h"
#include "tt_player.h"
#include "tt_collision.h"
#include "tt_items.h"
#include "tt_bullet.h"

/* At some point I'll see if I can prune these globals,
   but they're staying for now. */

bool dbg = false; 

bool program_done = false;
t_game game = { .state = LOAD_LEVEL, .next_state = LOAD_LEVEL };

const float FPS = 30;
int frame_speed = ANIMATION_SPEED;
int halftime_frame_speed = ANIMATION_SPEED * 2;

bool redraw = true;

#ifdef DEBUG
   double fps;
#endif

t_bullet player_bullet;

t_screen screen = { .unscaled_w = 320, .unscaled_h = 200 };

t_cam cam;
t_map *map = NULL;
t_player player = { .cur_frame = 0, .state = STOPPED, .vel_x = 4 };
t_item_afterfx *item_fx = NULL;

unsigned char item_frame = 0;
unsigned char item_afterfx_frame = 0;

bool key[14] = { false };

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


//Bitmaps that get drawn to
ALLEGRO_BITMAP *view_port = NULL;
//ALLEGRO_BITMAP *console_map = NULL;
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

ALLEGRO_SAMPLE_ID *snd_jump_id = NULL;


////Music
/* float mus_volume = 1;
ALLEGRO_SAMPLE *music = NULL;
ALLEGRO_SAMPLE_INSTANCE *music_instance = NULL; */

ALLEGRO_MIXER *music_mixer = NULL;
ALLEGRO_AUDIO_STREAM *music_stream = NULL;
struct ADL_MIDIPlayer *midi_player = NULL;

short buffer[1024];
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
   screen.width = 1280;
   screen.height = 720;
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
   al_lock_bitmap(border, al_get_bitmap_format(border), ALLEGRO_LOCK_READONLY);
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
      al_lock_bitmap(loading, al_get_bitmap_format(loading), ALLEGRO_LOCK_READONLY);
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

   //ADLMIDI
   midi_player = adl_init(22050);
   //adl_setTempo(midi_player, 0.40);
   adl_setVolumeRangeModel(midi_player, ADLMIDI_VolumeModel_APOGEE);
   adl_setLoopEnabled(midi_player, 1);
   
   
   adl_switchEmulator(midi_player, ADLMIDI_EMU_NUKED);

   #ifdef DEBUG
      al_set_window_title(display, "Tally Trauma -- DEBUG");
   #endif // DEBUG

   #ifdef RELEASE
      al_set_window_title(display, "Tally Trauma");
   #endif // RELEASE

   //Load font
   font = al_load_bitmap_font("data/fonts/font.png");
   fps_font = al_load_font("data/hack.ttf", 12, 0);

  

   //al_clear_to_color(al_map_rgb(0, 0, 0));
   //al_flip_display();


   if (adl_openFile(midi_player, "data/music/1.imf") < 0)
   {
      fprintf(stderr, "Couldn't open music file: %s\n", adl_errorInfo(midi_player));
      adl_close(midi_player);
      return 1;
   }

   music_stream = al_create_audio_stream(1, 1024, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_1);
   al_set_audio_stream_playing(music_stream, false);

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


   music_mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
   
   al_set_audio_stream_gain(music_stream, 5.0f);

   al_attach_audio_stream_to_mixer(music_stream, al_get_default_mixer());


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
   //al_set_target_backbuffer(display);
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
   al_lock_bitmap(stat_border, al_get_bitmap_format(stat_border), ALLEGRO_LOCK_READONLY);

   bg = al_load_bitmap("data/bg_1.png");
   if (bg == NULL)
   {
      jlog("Couldn't load bg1.png!");
      return false;
   }
   al_lock_bitmap(bg, al_get_bitmap_format(bg), ALLEGRO_LOCK_READONLY);

   tile_sheet = al_load_bitmap("data/tile_sheet.png");
   if (tile_sheet == NULL)
   {
      jlog("Couldn't load tile_sheet.png!");
      return false;
   }
   al_lock_bitmap(tile_sheet, al_get_bitmap_format(tile_sheet), ALLEGRO_LOCK_READONLY);

   item_sheet = al_load_bitmap("data/item_sheet.png");
   if (tile_sheet == NULL)
   {
      jlog("Couldn't load item_sheet.png!");
      return false;
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
         return false;
      }
      al_lock_bitmap(player.frame[j], al_get_bitmap_format(player.frame[j]), ALLEGRO_LOCK_READONLY);
      jlog("Player frame %d created and locked.", j);
   }

   item_fx_sheet = al_load_bitmap("data/item_score.png");
   if (item_fx_sheet == NULL) { jlog("Couldn't load item_score.png"); return false;; }
   al_lock_bitmap(item_fx_sheet, al_get_bitmap_format(item_fx_sheet), ALLEGRO_LOCK_READONLY);

   health_bar = al_load_bitmap("data/health_bar.png");
   if (health_bar == NULL) { jlog("Couldn't load health_bar.png"); return false; }
   al_lock_bitmap(health_bar, al_get_bitmap_format(health_bar), ALLEGRO_LOCK_READONLY);

   muzzle_flash = al_load_bitmap("data/muzzle_flash.png");
   if (muzzle_flash == NULL) { jlog("Couldn't load muzzle_flash.png"); return false; }
   al_lock_bitmap(muzzle_flash, al_get_bitmap_format(muzzle_flash), ALLEGRO_LOCK_READONLY);

   bullet_particle = al_load_bitmap("data/particle.png");
   if (bullet_particle == NULL) { jlog("Couldn't load particle.png"); return false; }
   al_lock_bitmap(bullet_particle, al_get_bitmap_format(bullet_particle), ALLEGRO_LOCK_READONLY);

   //Create viewport and console map bitmapsmusic_instance = al_create_sample_instance(music);
   //al_attach_sample_instance_to_mixer(music_instance, al_get_default_mixer());
   //al_set_sample_instance_gain(music_instance, mus_volume);
   //al_set_sample_instance_playmode(music_instance, ALLEGRO_PLAYMODE_LOOP);
   //al_play_sample_instance(music_instance);
   view_port = al_create_bitmap(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
//   console_map = al_create_bitmap(64, 45);

   //Load sounds
   snd_fall = al_load_sample("data/sound/fall.wav");
   snd_jump = al_load_sample("data/sound/jump.wav");
   snd_land = al_load_sample("data/sound/land.wav");
   snd_hithead = al_load_sample("data/sound/hithead.wav");
   snd_pickup = al_load_sample("data/sound/pickup.wav");
   snd_health = al_load_sample("data/sound/health.wav");
   snd_hurt = al_load_sample("data/sound/hurt.wav");
   snd_shoot = al_load_sample("data/sound/shoot.wav");

   /* music = al_load_sample(music_file); */

   //Play music (Make this into a function at some point.
   /* music_instance = al_create_sample_instance(music);
   al_attach_sample_instance_to_mixer(music_instance, al_get_default_mixer());
   al_set_sample_instance_gain(music_instance, mus_volume);
   al_set_sample_instance_playmode(music_instance, ALLEGRO_PLAYMODE_LOOP);
   al_play_sample_instance(music_instance); */

   al_set_audio_stream_playing(music_stream, true);

   //draw the status border
   al_set_target_bitmap(game_bmp);
   al_draw_bitmap(stat_border, 0, 0, 0);

   game.level_needs_unloaded = true;
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

   al_unlock_bitmap(tile_sheet);
   al_destroy_bitmap(tile_sheet);
   tile_sheet = NULL;
   if (tile_sheet == NULL) jlog("tile_sheet unloaded.");

   al_unlock_bitmap(item_sheet);
   al_destroy_bitmap(item_sheet);
   item_sheet = NULL;
   if (item_sheet == NULL) jlog("item_sheet unloaded.");

   al_unlock_bitmap(bg);
   al_destroy_bitmap(bg);
   bg = NULL;
   if (bg == NULL) jlog("bg unloaded.");

   al_unlock_bitmap(stat_border);
   al_destroy_bitmap(stat_border);
   stat_border = NULL;
   if (stat_border == NULL) jlog("stat_border unloaded.");

   al_unlock_bitmap(item_fx_sheet);
   al_destroy_bitmap(item_fx_sheet);
   item_fx_sheet = NULL;
   if (item_fx_sheet == NULL) jlog("item_fx_sheet unloaded.");

   al_unlock_bitmap(health_bar);
   al_destroy_bitmap(health_bar);
   health_bar = NULL;
   if (health_bar == NULL) jlog("health_bar unloaded.");

   al_unlock_bitmap(muzzle_flash);
   al_destroy_bitmap(muzzle_flash);
   muzzle_flash = NULL;
   if (muzzle_flash == NULL) jlog("muzzle_flash unloaded.");

   al_unlock_bitmap(bullet_particle);
   al_destroy_bitmap(bullet_particle);
   bullet_particle = NULL;
   if (bullet_particle == NULL) jlog("bullet_particle unloaded.");

   for (int j = 0; j < 8; ++j)
   {
      al_unlock_bitmap(player.frame[j]);
      al_destroy_bitmap(player.frame[j]);
      player.frame[j] = NULL;
      if (player.frame[j] == NULL) jlog("player.frame[%d] unloaded.", j);
   }

   al_unlock_bitmap(player.bitmap);
   al_destroy_bitmap(player.bitmap);
   player.bitmap = NULL;
   if (player.bitmap == NULL) jlog("player.bitmap unloaded.");


   al_destroy_bitmap(view_port);
   view_port = NULL;
   if (view_port == NULL) jlog("view_port unloaded.");

//   al_destroy_bitmap(console_map);
//   console_map = NULL;
//   if (console_map == NULL) jlog("console_map unloaded.");
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
 * The drawing function, called at redraw       *
 ************************************************/
void update_screen()
{
   draw_map(view_port, tile_sheet, item_sheet, bg, &cam, map, &item_frame);
   if (player.draw) draw_player(view_port, &cam, &player, player.direction);

   if (player.direction == RIGHT && player.muzzle_time) al_draw_bitmap(muzzle_flash, player.muzzle_x - cam.x, player.muzzle_y - cam.y, 0);
   if (player.direction == LEFT && player.muzzle_time) al_draw_bitmap(muzzle_flash, player.muzzle_x - cam.x, player.muzzle_y - cam.y, ALLEGRO_FLIP_HORIZONTAL);
   if (player_bullet.draw)
      al_draw_bitmap_region(bullet_particle, (player.muzzle_time) * 16, 0, 16, 16, player_bullet.end_x - cam.x - 8, player_bullet.end_y - cam.y - 8, 0);
      //al_draw_filled_circle(player_bullet.end_x - cam.x, player_bullet.end_y - cam.y, player.shoot_time /2, al_map_rgb(170, 0 ,0));

   draw_item_fx(view_port, item_fx_sheet, &cam, item_fx, &item_afterfx_frame, &player);
   
      #ifdef DEBUG
      if (dbg == true)
      {
         draw_bb(&cam, player.x + player.bb_left, player.y + player.bb_top, player.bb_width, player.bb_height);
         show_player_hotspot(view_port, &cam, &player);
      }
      #endif // DEBUG
//   draw_console_map(map, &player, console_map);
   #ifdef DEBUG
   if (dbg == true)
   {
      if (fps < 30) al_draw_textf(fps_font, al_map_rgb(255,0,0), 0, 0, ALLEGRO_ALIGN_LEFT, "FPS: %d", (int)fps);
      if (fps >= 30) al_draw_textf(fps_font, al_map_rgb(0,255,0), 0, 0, ALLEGRO_ALIGN_LEFT, "FPS: %d", (int)fps);
   }
   #endif
   //Draw view_port to game, then draw game scaled to display.
   al_set_target_bitmap(game_bmp);
    
   al_draw_bitmap(view_port, VIEWPORT_X, VIEWPORT_Y, 0);
//   al_draw_bitmap(console_map, 238, 100, 0);
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
         key[KEY_LCTRL] = true;
         break;
      case ALLEGRO_KEY_LSHIFT:
         key[KEY_LSHIFT] = true;
         break;
      case ALLEGRO_KEY_X:
         key[KEY_ALT] = true;
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
         key[KEY_LCTRL] = false;
         break;
      case ALLEGRO_KEY_LSHIFT:
         key[KEY_LSHIFT] = false;
         break;
      case ALLEGRO_KEY_X:
         key[KEY_ALT] = false;
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
         dbg ^= 1;
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
         if (is_ground(map, x, player_bullet.start_y))
         {
            player_bullet.end_x = x;
            player_bullet.end_y = player_bullet.start_y;
            printf("Bullet hit at %d, %d\n", player_bullet.end_x, player_bullet.end_y);
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
            printf("Bullet hit at %d, %d\n", player_bullet.end_x, player_bullet.end_y);
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
      player.bb_left = 13;
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
         play_sound(snd_land, false);
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
      play_sound(snd_fall, false);
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
   else if(player.on_ground == true && is_ground(map, player.x + x1, player.y + 32) && !is_ground(map, player.x +x3, player.y + 32) )
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
   if (key[KEY_LCTRL] && player.on_ground && !player.jump_pressed && !is_ground(map, player.x + x1, player.y-1) && !is_ground(map, player.x + x2, player.y-1))
   {
      if (!is_ground(map, player.x + x1, player.y-1) && !is_ground(map, player.x + x2, player.y-1))
      {
         play_sound(snd_jump, false);
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
      if(player.state == JUMPING)
      {
         play_sound(snd_hithead, false);
      }
      player.state = FALLING;

   }
   while (is_ground(map, player.x + x2, player.y))
   {
      player.y++;
      player.vel_y = 0;
      if(player.state == JUMPING)
      {
         play_sound(snd_hithead, false);
      }
      player.state = FALLING;

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

   //Collision test
   if (!player.hurt && check_collision(player.x + player.bb_left, player.y + player.bb_top, player.bb_width, player.bb_height, 304, 112, 16, 16))
   {
      play_sound(snd_hurt, false);
      player.hurt = PLAYER_HURT_TIME;
      if(player.health) player.health--;
   }
   if (player.hurt)
   {
      player.hurt--;
   }

   //Shooting time
   if (key[KEY_ALT] && !player.shoot_time)
   {
      check_bullet_collision();
      play_sound(snd_shoot, false);
      player.shoot_time = 5;
      player.muzzle_time = 4;
   }
   if (player.muzzle_time) player.muzzle_time--;
   else player_bullet.draw = false;

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

   check_cam();
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

   al_unlock_bitmap(loading);
   al_unlock_bitmap(border);

   adl_close(midi_player);

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

   if (argc > 1)
   {
      for (int d = 1; d < argc; d++)
       {
         if (strcmp(argv[d], "-tdbg") == 0)
         {
            dbg = true;
         }  
      }
   }
   printf("\n\ndbg = %d\n\n", dbg);
   

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

      double old_time = al_get_time();
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
      
      if (ev.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
      {

         samples_count = adl_play(midi_player, 1024, buffer);
         if (samples_count > 0)
         {
            al_set_audio_stream_fragment(music_stream, buffer);
         }
         else if (samples_count <= 0)
         {
            al_set_audio_stream_playing(music_stream, false);
         }

      }

      if (ev.type == ALLEGRO_EVENT_TIMER)
      {
         ++ticks;
         if (ticks == 1)
         {
            check_timer_logic();
            #ifdef DEBUG
            double new_time = al_get_time();
            double delta = new_time - old_time;
            fps = 1/(delta);
            old_time = new_time; 
            #endif 
            if (al_get_audio_stream_playing(music_stream)) printf("Playing\n");
            if (!al_get_audio_stream_playing(music_stream)) printf("NOT Playing\n");
            printf("Sample count: %d\n", samples_count);
         }
         redraw = true;
      }

      if (redraw && al_is_event_queue_empty(event_queue))
      {
         redraw = false;
         update_screen();
         ticks = 0;
      }
   }

   clean_up();

   return 0;
}
