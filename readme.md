Tally Trauma
============

<p align="center">
  <img src="http://suitorgames.com/images/tallyt.png" alt="Tally Screenshot" />
</p>

This is something I've been working on and haven't had time to
really do much else on it. Maybe I'm just bored with the project.
Anyway, I'm making the source code public.

It's an EGA-Styled retro platformer. Programmed in C with Allegro 5.

Please read the MIT license. All the art, music, and sound is made and owned 
by me, but if you wish to use it, or decide to do anything with the source, 
just give me some credit somewhere. And feel free to show me what you did,
I'd love to see it. 

I studied some code by Johan Pietz from Alex the Allegator 4, and I 
borrowed some concepts from that. It is noted as comments in the 
source files where appropriate.

I'm not liable for what this may do to you or anyone else's computer. So
don't blame me if things go awry! ;)

UPDATE 10-31-2019
-----------------
I've switched my main OS to Debian Buster because of REASONS, 
and I've finally successfully compiled this in linux. 

It has to be compiled in 32-bit for now because of pointer sizes
in the map data.

These are instructions for Debian Buster (10.1)

*In a terminal do the following
    
    `dpkg --add-architecture i386`
    `apt update`

*Use apt 
    
    `sudo apt liballegro5-dev:i386`
    `sudo apt liballegro5.2:i386`

*Make sure you have multilib GCC installed
    
    `sudo apt install g++-multilib gcc-multilib libc6-dev-i386`

*In the terminal set the environment variable below
    
    `export PKG_CONFIG_PATH=/usr/lib/i386-linux-gnu/pkgconfig`

*Clone the git repo
    
    `git clone https://github.com/knpfalcon/tally.git`

*CD to where you cloned the repo

*Compile with the long-ass command below (I didn't feel like making a makefile)
    
    `gcc jt_util.c tt_main.c tt_collision.c tt_items.c tt_map.c tt_player.c -o bin/tally $(pkg-config allegro-5 allegro_font-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_primitives-5  --libs --cflags) -m32`

-OR-

Simply use the shell script provided like so:   
    
    `./com.sh`

    `CD to bin`

    `type ./tally`

Excuse the mess
---------------
Sorry for all the mess... I'm working on maintaining this a little better.
I'm not a very organized person, and I work too fast for my own good.


Copyright © 2019 Joshua Daniel Taylor