#include <stdbool.h>

#include "tt_enemy.h"
#include "tt_map.h"
#include "tt_main.h"

void draw_enemies(t_map *m, t_enemy *e, t_cam *c, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (e[i].active == true)
        {
            if ( e[i].x > c->x - e[i].bb_width && 
             e[i].x < c->x + VIEWPORT_WIDTH + e[i].bb_width &&
             e[i].y > c->y - e[i].bb_height &&
             e[i].y < c->y + VIEWPORT_HEIGHT + e[i].bb_height)
            {
                al_draw_bitmap(e[i].frame[e[i].cur_frame], e[i].x - c->x, e[i].y - c->y, 0);
            }
        }
    }
}

void init_enemies(t_enemy *e)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        e[i].bitmap = NULL;
        
        for (int f = 0; f < MAX_ENEMY_FRAMES; f++)
        {
            e[i].frame[f] = NULL;
        }
        
        e[i].x = 0;
        e[i].y = 0;
        e[i].cur_frame = 0;
        e[i].direction = 0;
        e[i].bb_width = 16;
        e[i].bb_height = 16;
        e[i].active = false;

    }

}