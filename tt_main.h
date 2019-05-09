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

#define DISPLAY_MULTIPLIER 3

#define ANIMATION_SPEED 3

typedef struct t_game
{
   unsigned char state;

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

enum KEYS {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LCTRL, KEY_LSHIFT, KEY_ALT, KEY_Z, KEY_N, KEY_PAD_PLUS, KEY_PAD_MINUS, KEY_E, KEY_R};

extern bool key[13];

#endif // SP_MAIN_H
