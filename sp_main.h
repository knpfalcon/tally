#ifndef SP_MAIN_H
#define SP_MAIN_H

#include "jt_util.h"

//Macro hack for tile_sheet coordinates... It works, by God!
#define TILE000 448, 112
#define TILE001 400, 112
#define TILE002 128, 32

#define VIEWPORT_WIDTH 208
#define VIEWPORT_HEIGHT 160
#define VIEWPORT_X 16
#define VIEWPORT_Y 16

#define TILE_SIZE 16

#define MAP_WIDTH 128
#define MAP_HEIGHT 90

#define DISPLAY_MULTIPLIER 8

#define FONT_FILE "fixed_01.ttf"

typedef struct t_cam
{
   float x;
   float y;
} t_cam;

typedef struct t_mouse
{
   int x;
   int y;
   int over_tile_x;
   int over_tile_y;

}t_mouse;

#endif // SP_MAIN_H
