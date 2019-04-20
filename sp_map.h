#ifndef SP_MAP_H
#define SP_MAP_H

#include <stdbool.h>

// This is a position on the map
typedef struct t_map_pos
{
   bool empty_tile; //Does this slot have a tile? So we can use the full range of unsigned char (0h-FFh).
   unsigned char tile;//Tile to use. Tile sheets are 16x16 in tiles. 256 tiles a sheet.
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

#endif // SP_MAP_H
