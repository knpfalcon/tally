#ifndef TT_MAIN_H
#define TT_MAIN_H

#include <stdbool.h>
#include "jt_util.h"

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

enum STATE { QUIT, SPLASH, TITLE, MENU, LOAD_LEVEL, PLAY_LEVEL, QUIT_LEVEL };

enum DEMO_MODE { NONE, PLAY, RECORD };

#endif // SP_MAIN_H
