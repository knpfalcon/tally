#include "tt_player.h"

/*************************************************
 * Draws the player                              *
 *************************************************/
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

/*************************************************
 * Animates the player                           *
 *************************************************/
/* Instantly change from to the right frame, but
   only time the speed if there's more than one frame.
   This is so we can instantly change the frame, but
   time the animation. */
void animate_player(t_player *p, int *speed)
{
   if (p->state == JUMPING)
   {
      p->cur_frame = 5;
      p->muzzle_y = p->y + 9;
   }
   if (p->state == FALLING)
   {
      p->cur_frame = 6;
      p->muzzle_y = p->y + 9;
   }

   if (p->state == WALKING)
   {
      if (p->cur_frame == 0) p->cur_frame = 2;
      if (p->cur_frame > 4) p->cur_frame = 2;
      if (p->cur_frame < 5)
      {
         if (*speed <= 0)
         {
            *speed = ANIMATION_SPEED;
            p->cur_frame++;
         }
      }
      if (p->cur_frame == 5)
      {
         p->cur_frame = 1;
      }
   }

    //Set the muzzle flash position according to frame of player
   if (p->direction == RIGHT)
   {
      if (p->cur_frame == 1) { p->muzzle_x = p->x + 21; p->muzzle_y = p->y + 10; }
      else if (p->cur_frame == 3) { p->muzzle_x = p->x + 24; p->muzzle_y = p->y + 10; }
      else { p->muzzle_x = p->x + 23; p->muzzle_y = p->y + 9; }
   }
   if (p->direction == LEFT)
   {
      if (p->cur_frame == 1) { p->muzzle_x = p->x - 6; p->muzzle_y = p->y + 10; }
      else if (p->cur_frame == 3) { p->muzzle_x = p->x - 8; p->muzzle_y = p->y + 10; }
      else { p->muzzle_x = p->x + -7; p->muzzle_y = p->y + 9; }
   }

   if (p->state == STOPPED)
   {
      p->cur_frame = 0;
      p->muzzle_y = p->y + 9;
      if(p->muzzle_time) //If player shoots and is stopped set the muzzle xy and frame
      {
         p->cur_frame = 7;
         if (p->direction == RIGHT) p->muzzle_x = p->x + 21;
         if (p->direction == LEFT) p->muzzle_x = p->x -5;
      }

   }
}

/*************************************************
 * Debug stuff                                   *
 *************************************************/
#ifdef DEBUG
 /* Shows some pixels on the player */
void show_player_hotspot(ALLEGRO_BITMAP *bmp, t_cam *c, t_player *p)
{
   al_set_target_bitmap(bmp);
   al_draw_pixel(p->x + p->x1 - c->x, p->y + 32 - c->y, al_map_rgb(255,150,150));
   al_draw_pixel(p->x + p->x2 - c->x, p->y + 32 - c->y, al_map_rgb(150,255,150));
   al_draw_pixel(p->x + p->x3 - c->x, p->y + 32 - c->y, al_map_rgb(150,150,255));
}

#endif // DEBUG
