#ifndef SP_MAP_H
#define SP_MAP_H

#include <stdbool.h>

// This is a position on the map
typedef struct t_map_pos
{
   bool empty_tile;     //Does this slot have a tile?
   unsigned char tile;  //if empty_tile is false. What tile to use from a tile sheet? 0-255.
   unsigned char thing; //Does a thing reside in this slot? If so, what type? (Enemy, Door, etc.)
   unsigned char item;  //Is there an item in this slot? If so, What type? (Collectables, keys, etc.)
} t_map_pos;

//This is the map struct
typedef struct t_map
{
   char name[32];
   unsigned char bg;
   unsigned int num_enemies;
   int player_start_x;
   int player_start_y;
   t_map_pos *position;
} t_map;

t_map *create_empty_map();
void destroy_map(t_map *m);
bool save_map(t_map *m, ALLEGRO_DISPLAY *display);
t_map *load_map(const char *filename);
void draw_map(ALLEGRO_BITMAP *d_bmp, ALLEGRO_BITMAP *tile_sheet, ALLEGRO_BITMAP *background, t_cam *c, t_map *m);

#endif // SP_MAP_H
