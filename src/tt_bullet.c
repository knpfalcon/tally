#include "tt_bullet.h"


void animate_bullet_sparks(t_bullet_sparks *spark)
{
    for (int i = 0; i < MAX_SPARKS; i++)
    {
        if (spark[i].active)
        {
            spark[i].frame++;
            if (spark[i].frame >= 4)
            {
                spark[i].active = false;
                spark[i].frame = 0;
            }
        }
    }
}

void draw_bullet_spark(t_bullet_sparks *spark, ALLEGRO_BITMAP *src_bmp, t_cam *cam)
{
    for (int i = 0; i < MAX_SPARKS; i++)
    {
        if (spark[i].active)
        {
            al_draw_bitmap_region(src_bmp, spark[i].frame * 16, 0, 16, 16, spark[i].x - cam->x, spark[i].y - cam->y, 0);
        } 
    } 
}

void activate_bullet_spark(t_bullet_sparks *spark, int x, int y) // Find next unactive spark and activate it
{
    for (int i = 0; i < MAX_SPARKS; i++)
    {
        if (!spark[i].active)
        {
            spark[i].active = true;
            spark[i].x = x - 8;
            spark[i].y = y - 8;
            break;
        }
    }
}