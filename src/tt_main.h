#ifndef TT_MAIN_H
#define TT_MAIN_H

#include <stdbool.h>
#include "jt_util.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

#ifdef EDITOR
#define VIEWPORT_WIDTH 416
#define VIEWPORT_HEIGHT 320
#endif

#ifndef EDITOR
#define VIEWPORT_WIDTH 208
#define VIEWPORT_HEIGHT 160
#endif

#define VIEWPORT_X 16
#define VIEWPORT_Y 16

#define TILE_SIZE 16

#define MAP_WIDTH 128
#define MAP_HEIGHT 90

#define ANIMATION_SPEED 3

#define LEVEL_1 "data/maps/map.spl"

#define MUSIC_0 "data/music/music0.ogg"
#define MUSIC_1 "data/music/music1.ogg"

#define ORLO_STATE_NORMAL 0
#define ORLO_STATE_W_HEALTH 1

typedef struct t_game
{
   unsigned char state;
   unsigned char next_state;
   unsigned int episode;
   char *level;
   char *music;
   bool level_needs_unloaded;
   unsigned char demo_mode;
} t_game;

typedef struct t_cam
{
   int x;
   int y;
   int look_ahead;
} t_cam;

typedef struct t_screen
{
   int unscaled_w;
   int unscaled_h;
   int factor_x;
   int factor_y;
   int factor;
   int x;
   int y;
   int width;
   int height;
} t_screen;

typedef struct 
{
   ALLEGRO_BITMAP *loading;
   ALLEGRO_BITMAP *border;
   ALLEGRO_BITMAP *tile_sheet;
   ALLEGRO_BITMAP *item_sheet;
   ALLEGRO_BITMAP *bg;
   ALLEGRO_BITMAP *stat_border;
   ALLEGRO_BITMAP *item_fx_sheet;
   ALLEGRO_BITMAP *health_bar;
   ALLEGRO_BITMAP *bullet_blue;
   ALLEGRO_BITMAP *muzzle_flash;
   ALLEGRO_BITMAP *bullet_particle;
   ALLEGRO_BITMAP *laser;
   ALLEGRO_BITMAP *bad_robot_1;
} t_graphics;

typedef struct
{
   ALLEGRO_SAMPLE *pickup;
   ALLEGRO_SAMPLE *hurt;
   ALLEGRO_SAMPLE *health;
   ALLEGRO_SAMPLE *fall;
   ALLEGRO_SAMPLE *jump;
   ALLEGRO_SAMPLE *land;
   ALLEGRO_SAMPLE *hithead;
   ALLEGRO_SAMPLE *shoot;
   ALLEGRO_SAMPLE *orlo_give_health;
   ALLEGRO_SAMPLE *orlo_get_health;
}t_sounds;


enum STATE { QUIT, SPLASH, TITLE, MENU, LOAD_LEVEL, PLAY_LEVEL, QUIT_LEVEL };

enum DEMO_MODE { NONE, PLAY, RECORD };

#endif // SP_MAIN_H
