#include "tt_player.h"


void draw_player(ALLEGRO_BITMAP *bmp, t_cam *c, t_player *p)
{
   al_set_target_bitmap(bmp);

   if ((c->x - 32) < p->x  &&
       (c->y - 32) < p->y  &&
       (c->x + VIEWPORT_WIDTH) > p->x &&
       (c->y + VIEWPORT_HEIGHT) > p->y)
   {
      al_draw_bitmap(p->frame[p->cur_frame], p->x - c->x, p->y - c->y, 0);
   }
}

void check_player_animation(t_player *p)
{
   if (p->state == GROUNDED)
   {
      p->cur_frame = 0;
   }
   else if (p->state == WALKING)
   {
      if (p->cur_frame < 5)
      {
         p->cur_frame++;
      }

      if (p->cur_frame == 5)
      {
         p->cur_frame = 1;
      }
   }


}
