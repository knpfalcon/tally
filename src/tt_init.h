#ifndef _TT_INIT_H
#define _TT_INIT_H

#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <adlmidi.h>
#include "tt_main.h"

bool init_game( ALLEGRO_DISPLAY **display, t_screen *screen, ALLEGRO_EVENT_QUEUE **event_queue, ALLEGRO_TIMER **FPS_TIMER, float FPS, 
                t_graphics *graphics, ALLEGRO_FONT **font_status, ALLEGRO_FONT **font_message, struct ADL_MIDIPlayer **midi_player,
                long STREAM_FREQ, unsigned int BUFFER_SAMPLES, ALLEGRO_AUDIO_STREAM **music_stream, t_game *game, ALLEGRO_BITMAP **game_bmp );

#endif