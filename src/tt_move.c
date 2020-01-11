#include "tt_move.h"

void check_horizontal_tile_collision(t_map *map, void *thing, int x1, int x2, int old_x)
{
   if (((t_thing*)thing)->y + 32 > 0)
      {
         if (is_ground(map, ((t_thing*)thing)->x + x1, ((t_thing*)thing)->y + 2 )) ((t_thing*)thing)->x = old_x; //top
         if (is_ground(map, ((t_thing*)thing)->x + x2, ((t_thing*)thing)->y + 2 )) ((t_thing*)thing)->x = old_x;

         if (is_ground(map, ((t_thing*)thing)->x + x1, ((t_thing*)thing)->y + 16 )) ((t_thing*)thing)->x = old_x; //center
         if (is_ground(map, ((t_thing*)thing)->x + x2, ((t_thing*)thing)->y + 16 )) ((t_thing*)thing)->x = old_x;

         if (is_ground(map, ((t_thing*)thing)->x + x1, ((t_thing*)thing)->y + 31 )) ((t_thing*)thing)->x = old_x; //bottom
         if (is_ground(map, ((t_thing*)thing)->x + x2, ((t_thing*)thing)->y + 31 )) ((t_thing*)thing)->x = old_x;
      }
}

bool return_horizontal_tile_collision(t_map *map, void *thing, int x1, int x2)
{
   if (((t_thing*)thing)->y + 32 > 0)
      {
         if (is_ground(map, ((t_thing*)thing)->x + x1, ((t_thing*)thing)->y + 2 )) return true; //top
         if (is_ground(map, ((t_thing*)thing)->x + x2, ((t_thing*)thing)->y + 2 )) return true;

         if (is_ground(map, ((t_thing*)thing)->x + x1, ((t_thing*)thing)->y + 16 )) return true; //center
         if (is_ground(map, ((t_thing*)thing)->x + x2, ((t_thing*)thing)->y + 16 )) return true;

         if (is_ground(map, ((t_thing*)thing)->x + x1, ((t_thing*)thing)->y + 31 )) return true; //bottom
         if (is_ground(map, ((t_thing*)thing)->x + x2, ((t_thing*)thing)->y + 31 )) return true;
      }
   return false;
}

void check_ceiling(t_map *map, void *thing, int x1, int x2)
{
    while (is_ground(map, ((t_thing*)thing)->x + x1, ((t_thing*)thing)->y))
      {
         ((t_thing*)thing)->y++;
         ((t_thing*)thing)->vel_y = 0;
         if( ((t_thing*)thing)->state == JUMPING)
         {
            //play_sound(sounds->hithead, false);
         }
         ((t_thing*)thing)->state = FALLING;

      }
      while (is_ground(map, ((t_thing*)thing)->x + x2, ((t_thing*)thing)->y))
      {
         ((t_thing*)thing)->y++;
         ((t_thing*)thing)->vel_y = 0;
         if( ((t_thing*)thing)->state == JUMPING)
         {
            //play_sound(sounds->hithead, false);
         }
         ((t_thing*)thing)->state = FALLING;
      }
}

void check_floor(t_map *map, void *thing, int x1, int x2)
{
   while (is_ground(map, ((t_thing*)thing)->x + x1, ((t_thing*)thing)->y + 31) && ((t_thing*)thing)->y + 31 > 0)
   {
      ((t_thing*)thing)->y--;
      ((t_thing*)thing)->on_ground = true;
   }
   while (is_ground(map, ((t_thing*)thing)->x + x2, ((t_thing*)thing)->y + 31) && ((t_thing*)thing)->y + 31 > 0)
   {
      ((t_thing*)thing)->y--;
      ((t_thing*)thing)->on_ground = true;
   }
}

void apply_gravity(void *thing, int force)
{
   ((t_thing*)thing)->y += ((t_thing*)thing)->vel_y / 4;
   if ( ((t_thing*)thing)->vel_y > force) ((t_thing*)thing)->vel_y = force;
}

void jump(void *thing, int strength)
{
   ((t_thing*)thing)->vel_y = -(strength);
}