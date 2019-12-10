#ifndef TT_MAIN_H
#define TT_MAIN_H

#include "jt_util.h"

#define VIEWPORT_WIDTH 208
#define VIEWPORT_HEIGHT 160
#define VIEWPORT_X 16
#define VIEWPORT_Y 16

#define TILE_SIZE 16

#define MAP_WIDTH 128
#define MAP_HEIGHT 90

#define ANIMATION_SPEED 3

#define LEVEL_1 "data/maps/map.spl"

#define MUSIC_0 "data/music/music0.ogg"
#define MUSIC_1 "data/music/music1.ogg"

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
   float x;
   float y;
   float look_ahead;
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

enum KEYS {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_Z, KEY_LSHIFT, KEY_X, KEY_T, KEY_N, KEY_PAD_PLUS, KEY_PAD_MINUS, KEY_E, KEY_R, KEY_Q, KEY_F};

extern bool key[15];

#endif // SP_MAIN_H
