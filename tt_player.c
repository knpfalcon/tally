
#include "memwatch/memwatch.h"
#include "tt_player.h"

void draw_player(ALLEGRO_BITMAP *bmp, t_cam *c, t_player *p, unsigned char direction)
{
   al_set_target_bitmap(bmp);

   if ((c->x - 32) < p->x  &&
       (c->y - 32) < p->y  &&
       (c->x + VIEWPORT_WIDTH) > p->x &&
       (c->y + VIEWPORT_HEIGHT) > p->y)
   {
      if (direction == RIGHT)
         al_draw_bitmap(p->frame[p->cur_frame], p->x - c->x, p->y - c->y, 0);
      else
         al_draw_bitmap(p->frame[p->cur_frame], p->x - c->x, p->y - c->y, ALLEGRO_FLIP_HORIZONTAL);
   }
}

void animate_player(t_player *p)
{
   if (p->state == JUMPING)
   {
      p->cur_frame = 5;
   }
   if (p->state == FALLING)
   {
      p->cur_frame = 6;
   }

   if (p->state == WALKING)
   {
      if (p->cur_frame == 0) p->cur_frame = 2;
      if (p->cur_frame > 4) p->cur_frame = 2;
      if (p->cur_frame < 5)
      {
         p->cur_frame++;
      }
      if (p->cur_frame == 5)
      {
         p->cur_frame = 1;
      }

   }
   if (p->state == STOPPED)
   {
      p->cur_frame = 0;
   }

}

void show_player_hotspot(ALLEGRO_BITMAP *bmp, t_cam *c, t_player *p)
{
   al_set_target_bitmap(bmp);
   al_lock_bitmap(bmp, al_get_bitmap_format(bmp), ALLEGRO_LOCK_READWRITE);
   //al_put_pixel(p->hotspot_x - c->x, p->hotspot_y - c->y, al_map_rgb(255,0,0));
   al_unlock_bitmap(bmp);
}

