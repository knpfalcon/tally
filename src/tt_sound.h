#ifndef TT_SOUND_H
#define TT_SOUND_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>

extern float sfx_volume;

void play_sound(ALLEGRO_SAMPLE *s, bool interupt);

#endif