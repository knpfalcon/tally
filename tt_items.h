#ifndef TT_ITEMS_H
#define TT_ITEMS_H

#include "tt_map.h"

typedef struct t_itemfx_index
{
   bool active;
   unsigned char type;
   int x;
   int y;
   t_map_pos *position;
   int life_span;
} t_itemfx_index;

typedef struct t_item_afterfx
{
   int item_count;
   t_itemfx_index *index;
} t_item_afterfx;

t_item_afterfx *create_item_after_fx(t_map *m);
void destroy_item_afterfx(t_item_afterfx *ifx);
void draw_item_fx(ALLEGRO_BITMAP *bmp, ALLEGRO_BITMAP *sheet, t_cam *c, t_item_afterfx *ifx, unsigned char *item_frame);
void activate_item_fx(t_map_pos *mp, t_item_afterfx *ifx);
void update_item_afterfx(t_item_afterfx *ifx);
#endif // TT_ITEMS_H
