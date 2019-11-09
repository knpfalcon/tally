#ifndef SP_EDIT_H
#define SP_EDIT_H

#define FONT_FILE "data/hack.ttf"

#define DISPLAY_MULTIPLIER 3

typedef struct t_conditional
{
   bool show_mini_map;
   bool map_load;
   bool name_map;
   bool map_saved;

} t_conditional;

typedef struct t_mouse
{
   int x;
   int y;
   int z;
   int over_tile_x;
   int over_tile_y;
   unsigned char tile_selection;
} t_mouse;


#endif // SP_EDIT_H
