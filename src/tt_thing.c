#include <stdbool.h>

#include "tt_thing.h"
#include "tt_map.h"
#include "tt_main.h"

void draw_things(t_map *m, t_thing *t, t_cam *c, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (t[i].active == true)
        {
            if ( t[i].x > c->x - t[i].width && 
             t[i].x < c->x + VIEWPORT_WIDTH &&
             t[i].y > c->y - t[i].height &&
             t[i].y < c->y + VIEWPORT_HEIGHT)
            {
                al_draw_bitmap(t[i].frame[t[i].cur_frame], t[i].x - c->x, t[i].y - c->y, 0);
            }
        }
    }
}

void clear_things(t_thing *t)
{
    for (int i = 0; i < MAX_THINGS; i++)
    {
        t[i].bitmap = NULL;
        for (int f = 0; f < MAX_ENEMY_FRAMES; f++)
        {
            t[i].frame[f] = NULL;
        }
        t[i].x = 0;
        t[i].y = 0;
        t[i].cur_frame = 0;
        t[i].direction = 0;
        t[i].state = 0;
        t[i].bb_width = 0;
        t[i].bb_height = 0;
        t[i].bb_width = 0;
        t[i].active = false;
    }
}

//Loads things into the game (NOT THE EDITOR)
void load_things(t_thing *t, t_map *m)
{
    clear_things(t);

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            if (m->position[x + y * MAP_WIDTH].thing > 0 && m->num_things < MAX_THINGS)
            {
                m->num_things++;
                t[m->num_things -1].active  = true;
                t[m->num_things -1].x = x * 16;
                t[m->num_things -1].y = y * 16;
                t[m->num_things -1].touched = false;
            }

            if (m->position[x + y * MAP_WIDTH].thing == ENEMY_SPIKES)
            {
                t[m->num_things -1].frame[0] = al_load_bitmap("data/spikes.png");
                t[m->num_things -1].type = ENEMY_SPIKES;
                t[m->num_things -1].bb_height = 16;
                t[m->num_things -1].bb_width = 16;
                t[m->num_things -1].bb_top = 0;
                t[m->num_things -1].bb_left = 0;
                t[m->num_things -1].width = 16;
                t[m->num_things -1].height = 16;
            }

            if (m->position[x + y * MAP_WIDTH].thing == THING_ORLO)
            {
                t[m->num_things -1].bitmap = al_load_bitmap("data/orlo.png");
                for (int i =0; i < 5; i++)
                {
                    t[m->num_things -1].frame[i] = al_create_sub_bitmap(t[m->num_things -1].bitmap, i * 32, 0, 32, 32);
                }
                
                t[m->num_things -1].type = THING_ORLO;
                t[m->num_things -1].bb_height = 32;
                t[m->num_things -1].bb_width = 12;
                t[m->num_things -1].bb_top = 0;
                t[m->num_things -1].bb_left = 9;
                t[m->num_things -1].width = 32;
                t[m->num_things -1].height = 32;
            }
            
        }
    }
}
