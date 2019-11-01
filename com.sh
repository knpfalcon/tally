gcc jt_util.c tt_main.c tt_collision.c tt_items.c tt_map.c tt_player.c -o bin/tally $(pkg-config allegro-5 allegro_font-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_primitives-5  --libs --cflags) -m32 -O2 -pedantic -Werror -fomit-frame-pointer -fexpensive-optimizations -std=c99 -g

echo "Done"
