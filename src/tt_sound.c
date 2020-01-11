#include "tt_sound.h"
/*
 * Plays a sound without all the boilerplate
 */
void play_sound(ALLEGRO_SAMPLE *s, bool interupt)
{
   if (interupt) al_stop_samples();
   al_play_sample(s, sfx_volume, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
}