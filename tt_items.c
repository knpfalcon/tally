
#include "tt_items.h"

t_item_afterfx *create_item_after_fx(t_map *m)
{
   int item_count = 0;
   int it = 0;

   t_item_afterfx *ifx = NULL;

   //check how many items are in map
   for(int y = 0; y < MAP_HEIGHT; y++)
   {
      for(int x = 0; x < MAP_WIDTH; x++)
      {
         if (m->position[x+y*MAP_WIDTH].item > 0)
         {
            item_count++;
         }
      }
   }

   //Allocation of item after effects according to how many items there are
   ifx = malloc(sizeof(t_item_afterfx));
   if (ifx == NULL) return NULL;
   ifx->item_count = item_count;
   jlog("%d items set for item_count.", ifx->item_count);

   ifx->index = malloc(item_count * sizeof(t_itemfx_index));

   //Check the map again, and match up items with the after effect
   for(int y = 0; y < MAP_HEIGHT; y++)
   {
      for(int x = 0; x < MAP_WIDTH; x++)
      {
         if (m->position[x+y*MAP_WIDTH].item > 0)
         {
            if (it < item_count)
            {
               ifx->index[it].active = false;
               ifx->index[it].position = get_map_position(m, x * 16, y *16);
               ifx->index[it].x = x * TILE_SIZE;
               ifx->index[it].y = y * TILE_SIZE;
               ifx->index[it].life_span = 32;
               ifx->index[it].type = m->position[x+y*MAP_WIDTH].item;
               m->position[x+y*MAP_WIDTH].item_index = it;
               jlog("Item %d: Active? %d, X? %d, Y? %d, LIFESPAN? %d", it, ifx->index[it].active, ifx->index[it].x, ifx->index[it].y, ifx->index[it].life_span);
               it++;
            }
         }
      }
   }

   return ifx;
}

void destroy_item_afterfx(t_item_afterfx *ifx)
{
   if (ifx->index == NULL) return;
   free(ifx->index);

   if (ifx == NULL) return;

   free(ifx);
   jlog("Item after effects destroyed.");

}

void draw_item_fx(ALLEGRO_BITMAP *bmp, ALLEGRO_BITMAP *sheet, t_cam *c, t_item_afterfx *ifx, unsigned char *item_frame, t_player *p)
{
   al_set_target_bitmap(bmp);

   //scan for active item after effects
   for (int i = 0; i < ifx->item_count; i++)
   {
      if (ifx->index[i].active)
      {
         if ((c->x - 16) < ifx->index[i].x  && (c->y - 16) < ifx->index[i].y  && (c->x + VIEWPORT_WIDTH) > ifx->index[i].x && (c->y + VIEWPORT_HEIGHT) > ifx->index[i].y)
         {
            if (ifx->index[i].type < ITEM_VHS) // 100 points
            {
               al_draw_bitmap_region(sheet, 0, *item_frame * 16, 16, 16, ifx->index[i].x - c->x, (ifx->index[i].y) - c->y, 0);
            }
            else if (ifx->index[i].type >= ITEM_VHS && ifx->index[i].type < ITEM_UNDERWEAR) // 200 points
            {
               al_draw_bitmap_region(sheet, 16, *item_frame * 16, 16, 16, ifx->index[i].x - c->x, (ifx->index[i].y) - c->y, 0);
            }
            else if (ifx->index[i].type >= ITEM_UNDERWEAR && ifx->index[i].type < ITEM_WRENCH) // 500 points
            {
               al_draw_bitmap_region(sheet, 32, *item_frame * 16, 16, 16, ifx->index[i].x - c->x, (ifx->index[i].y) - c->y, 0);
            }
            else if (ifx->index[i].type >= ITEM_WRENCH && ifx->index[i].type < ITEM_MONEY) // 1000 points
            {
               al_draw_bitmap_region(sheet, 48, *item_frame * 16, 16, 16, ifx->index[i].x - c->x, (ifx->index[i].y) - c->y, 0);
            }
            else if (ifx->index[i].type == ITEM_MONEY) //2000 points
            {
               al_draw_bitmap_region(sheet, 64, *item_frame * 16, 16, 16, ifx->index[i].x - c->x, (ifx->index[i].y) - c->y, 0);
            }
            else if (ifx->index[i].type == ITEM_DIAMOND) //5000 points
            {
               al_draw_bitmap_region(sheet, 80, *item_frame * 16, 16, 16, ifx->index[i].x - c->x, (ifx->index[i].y) - c->y, 0);
            }
            else if (ifx->index[i].type == ITEM_HEALTH) // Health
            {
               al_draw_bitmap_region(sheet, 96, *item_frame * 16, 16, 16, ifx->index[i].x - c->x, (ifx->index[i].y) - c->y, 0);
            }
         }
      }
   }
}

void update_item_afterfx(t_item_afterfx *ifx)
{
   for (int i = 0; i < ifx->item_count; i++)
   {
      if (ifx->index[i].active)
      {
         ifx->index[i].life_span--;
         if (ifx->index[i].life_span > 0) ifx->index[i].y--;
         else if( ifx->index[i].life_span == 0) ifx->index[i].active = false;
      }
   }
}
void activate_item_fx(t_map_pos *mp, t_item_afterfx *ifx)
{

   ifx->index[mp->item_index].active = true;

   if (ifx->index[mp->item_index].active == true) jlog("Item %d activated.", mp->item_index);
}
