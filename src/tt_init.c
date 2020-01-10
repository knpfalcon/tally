#include <stdio.h>
#include "tt_init.h"

/*
 * Initiate everything the game needs at startup 
 */
bool init_game( ALLEGRO_DISPLAY **display, t_screen *screen, ALLEGRO_EVENT_QUEUE **event_queue, ALLEGRO_TIMER **FPS_TIMER, float FPS, 
                t_graphics *graphics, ALLEGRO_FONT **font_status, ALLEGRO_FONT **font_message, struct ADL_MIDIPlayer **midi_player,
                long STREAM_FREQ, unsigned int BUFFER_SAMPLES, ALLEGRO_AUDIO_STREAM **music_stream, t_game *game, ALLEGRO_BITMAP **game_bmp )
{
   
   //Initialize Allegro
   if(!al_init())
   {
      jlog("Failed to initialize allegro!\n");
      return false;
   }
   uint32_t version = al_get_allegro_version();
   int major = version >> 24;
   int minor = (version >> 16) & 255;
   int revision = (version >> 8) & 255;
   int release = version & 255;
   jlog("Allegro %d.%d.%d release %d", major, minor, revision, release);

   if (!al_init_image_addon())
   {
      jlog("Failed to initialize image_addon!");
      return false;
   }
   jlog("Image add-on initialized.");
   //al_set_new_display_adapter(1);
   screen->width = 1280;
   screen->height = 800;
   //al_set_new_display_flags(ALLEGRO_NOFRAME);
   al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
   *display = al_create_display(screen->width, screen->height);
   if(!display)
   {
      jlog("Failed to create display!");
      return false;
   }
   //Special thanks to dthompson from the allegro.cc forums for the setup below
   screen->factor_x = al_get_display_width(*display) / screen->unscaled_w;
   screen->factor_y = al_get_display_height(*display) / screen->unscaled_h;
   screen->factor = (screen->factor_y < screen->factor_x) ? screen->factor_y : screen->factor_x;
   screen->width = screen->unscaled_w * screen->factor;
   screen->height = screen->unscaled_h * screen->factor;
   screen->x = (al_get_display_width(*display) / 2) - (screen->width/2);
   screen->y = (al_get_display_height(*display) / 2) - (screen->height/2);
   jlog("Display Created.");
   
   // Load/Draw the fill-in border
   graphics->border = al_load_bitmap("data/bg_border.png");
   if (graphics->border == NULL)
   {
      jlog("Couldn't load bg_border.png!");
      return false;
   }
   /* for (int y = 0; y < (al_get_display_height(display) / (32 * screen.factor)) + 1; y++)
   {
      for (int x = 0; x < (al_get_display_width(display) / (32 * screen.factor)) + 1; x++)
      {
         al_draw_scaled_bitmap(border, 0, 0, 32, 32, x * (32 * screen.factor), y * (32 * screen.factor), 32 * screen.factor, 32 * screen.factor, 0);
      }
   } */
   //Load/Draw Loading bitmap and flip the display
   graphics->loading = al_load_bitmap("data/loading.png");
   if (graphics->loading == NULL)
   {
      jlog("Couldn't load loading.png!");
   }
   else
   {
      al_draw_scaled_bitmap(graphics->loading, 0, 0, screen->unscaled_w, screen->unscaled_h, screen->x, screen->y, screen->width, screen->height, 0);
      al_flip_display();
   }

   *event_queue = al_create_event_queue();
   if (!*event_queue)
   {
      jlog("Failed to create event queue!");
      return false;
   }
   jlog("Event queue created.");

   *FPS_TIMER = al_create_timer(ALLEGRO_BPS_TO_SECS(FPS));
   if(!*FPS_TIMER)
   {
      jlog("Failed to create FPS timer!");
      return false;
   }
   jlog("FPS timer created.");

   if (!al_install_keyboard())
   {
      jlog("Failed to install keyboard!");
      return false;
   }
   jlog("Keyboard installed.");

   //Font Add-ons
   if(!al_init_font_addon())
   {
      jlog("Failed to install fonts addon!");
      return false;
   }
   //Font Add-ons
   if(!al_init_ttf_addon())
   {
      jlog("Failed to install ttf addon!");
      return false;
   }
   jlog("Fonts add-on installed.");

   //Primitives
  /*  if (!al_init_primitives_addon())
   {
      jlog("Failed to install primitives addon!");
      return false;
   }
   jlog("Primitives add-on initialized."); */

   //Audio
   if (!al_install_audio())
   {
      jlog("Failed to install audio!");
      return false;
   }
   if (!al_init_acodec_addon())
   {
      jlog("Failed to initialize audio codec addon!");
      return false;
   }
   jlog("Audio initialized.");

   #ifdef DEBUG
   al_set_window_title(display, "Tally Trauma -- DEBUG");
   #endif // DEBUG

   #ifdef RELEASE
   al_set_window_title(display, "Tally Trauma");
   #endif // RELEASE

   //Load font
   *font_status = al_load_bitmap_font("data/fonts/font.png");
   *font_message = al_load_bitmap_font("data/fonts/pixfont.png");

   //ADLMIDI
   *midi_player = adl_init(STREAM_FREQ);
   adl_setLoopEnabled(*midi_player, 1);
   adl_switchEmulator(*midi_player, ADLMIDI_EMU_DOSBOX);
   adl_openBankFile(*midi_player, "data/gm.wopl");
   if (adl_openFile(*midi_player, "data/music/dsong.mid") < 0)
   {
      jlog("Couldn't open music file: %s", adl_errorInfo(*midi_player));
      adl_close(*midi_player);
      return 1;
   }

   *music_stream = al_create_audio_stream(2, BUFFER_SAMPLES, STREAM_FREQ, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
   
   al_register_event_source(*event_queue, al_get_display_event_source(*display));
   al_register_event_source(*event_queue, al_get_timer_event_source(*FPS_TIMER));
   al_register_event_source(*event_queue, al_get_keyboard_event_source());
   al_register_event_source(*event_queue, al_get_audio_stream_event_source(*music_stream));

   //Create the game bitmap that needs to be stretched to display
   *game_bmp = al_create_bitmap(320, 200);

   al_reserve_samples(8);

   jlog("Game initialized.");
   jlog("Screen size factor: %d", screen->factor);

   game->level = LEVEL_1;
   /* game.music = MUSIC_1; */
   al_set_mixer_gain(al_get_default_mixer(), 1.0f); //Turn down the volume during development
   al_set_audio_stream_gain(*music_stream, 2.0f);
   al_attach_audio_stream_to_mixer(*music_stream, al_get_default_mixer());
   al_set_audio_stream_playing(*music_stream, false);
   jlog("DEPTH: %d\n", al_get_mixer_depth(al_get_default_mixer()));
   
   return true;
}