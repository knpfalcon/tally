#ifndef SP_MAIN_H
#define SP_MAIN_H

#include "jt_util.h"

#define VIEWPORT_WIDTH 208
#define VIEWPORT_HEIGHT 160
#define VIEWPORT_X 16
#define VIEWPORT_Y 16

#define TILE_SIZE 16

#define MAP_WIDTH 128
#define MAP_HEIGHT 90

#define DISPLAY_MULTIPLIER 10

typedef struct t_cam
{
   float x;
   float y;
} t_cam;

enum KEYS {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LCTRL, KEY_LSHIFT, KEY_ALT};


#endif // SP_MAIN_H
