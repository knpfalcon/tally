#ifndef SP_MAP_H
#define SP_MAP_H

#include <stdbool.h>

// This is a position on the map
typedef struct t_map_pos
{
   bool empty_tile; //Does this slot have a tile? So we can use the full range of unsigned char (0h-FFh).
   unsigned char tile;
//   unsigned char tile_x;//Tile's X position to use on tile sheet.
//   unsigned char tile_y;//Tile's Y position to use on tile sheet.
   unsigned char thing;//What thing is in this slot? (Enemy, door, switch, etc.)
   unsigned char item; //Is there an item in this slot? (Collectibles, keys, weapons, ammo.)
} t_map_pos;

typedef struct t_map
{
   char name[32];
   unsigned char bg;
   t_map_pos *position;

} t_map;

t_map *create_empty_map();
void destroy_map(t_map *m);
bool save_map(t_map *m, ALLEGRO_DISPLAY *display);
t_map *load_map(const char *filename);
void draw_map(ALLEGRO_BITMAP *d_bmp, ALLEGRO_BITMAP *tiles[], ALLEGRO_BITMAP *background, t_cam *c, t_map *m);

#endif // SP_MAP_H
