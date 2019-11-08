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

**Warning**
This game is far from finished, and the editor had many errors.

I'm not liable for what this may do to you or anyone else's computer. So
don't blame me if things go awry! ;)

Linux
-----------------
I've switched my main OS to Debian Buster because of REASONS, 
and I've finally successfully compiled this in linux. 

It has to be compiled in 32-bit for now because of pointer sizes
in the map data.

These are instructions for **Debian Buster (10.1)**

In a terminal do the following
    
    dpkg --add-architecture i386
    apt update

Install the 32-bit Allegro libraries
    
    sudo apt liballegro5-dev:i386 liballegro5.2:i386

Make sure you have multilib GCC installed
    
    sudo apt install g++-multilib gcc-multilib libc6-dev-i386

In the terminal set the environment variable below
    
    export PKG_CONFIG_PATH=/usr/lib/i386-linux-gnu/pkgconfig

Clone the git repo
    
    git clone https://github.com/knpfalcon/tally.git
    cd tally

Compile the editor and the game
    make
    cd bin

To run the game
    ./tally

To run the editor
    ./tedit


Excuse the mess
---------------
Sorry for all the mess... I'm working on maintaining this a little better.
I'm not a very organized person, and I work too fast for my own good.


Copyright © 2019 Joshua Daniel Taylor