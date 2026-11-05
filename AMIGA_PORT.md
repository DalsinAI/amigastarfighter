Project: Starfighter 2.4 for AmigaOS 3.x
========================================

Project: Starfighter (https://github.com/pr-starfighter/starfighter), the
2D space shooter by Parallel Realities, Guus Sliepen, Astrid S. de Wijn
and others, for AmigaOS 3.x on a 68040 or 68060 with an FPU and an RTG
screen, on OpenGPU's SDL 2.

Based on upstream tag v2.4.0-1 (25c523c0db0ca7ff625ab8a9cc713cab234a1aeb).

Needs
-----
- AmigaOS 3.x, 68040/68060 with an FPU (not an LC040/EC040), RTG.
- OpenGPU's runtime (OpenUp installs it): opengpu.library and
  LIBS:OpenGPU/SDL2.module, SDL2_image.module and SDL2_mixer.module
  (the PNG pictures and the Ogg Vorbis sound and music).
- About 46 MB of disk for the drawer.

How to play
-----------
Double-click Starfighter in its drawer, or from a Shell:

    cd Starfighter
    Starfighter            (Starfighter -noaudio for no sound)

The arrow keys move, Ctrl fires, Space fires the secondary weapon, Shift
(or Alt) switches the fire mode, P pauses and Escape is the menu. The keypad, Z X C, A S D,
< > ? and 1 2 3 work too, and so does a joystick or gamepad. In the
intermission screens the arrows move the pointer and fire "clicks".

The settings (conf) and the saved games (saveNN.sav, text files) go in the
drawer's Settings drawer. The full-screen option is in the title menu's
options (and F11).

What was done
-------------
- amiga/Makefile.amigaos: builds the program and the drawer
  dist/Starfighter (+ Starfighter.info), as upstream's configure builds it
  without SDL_ttf and Pango: -DNOFONT, the game's own bitmap font
  (gfx/smallFont.png) and English text.
- amiga/libintl.h: no gettext; the text stays as written (English).
- src/engine.c: the settings and saves go in PROGDIR:Settings/ (no HOME,
  no getpwuid on AmigaOS); amiga/amiga_screens.c (from the Seven Kingdoms
  port) closes the empty "SDL" screens SDL2.module 0.6 leaves behind after
  a full-screen quit.
- Data from PROGDIR: (DATADIR), so the game finds gfx/, sound/, music/ and
  data/ when started from any Shell directory or the Workbench.
- The window is upstream's smallest, 640x480 (SCREEN_W/SCREEN_H in the
  Makefile change it; upstream's default is 800x600): less to draw and
  upload per frame on a 68k.
- amiga/stack.c: 256 KB of stack (libnix's swapstack); the tool icon asks
  for the same.
- Every save and settings file is text (fscanf/fprintf), so there is no
  byte-order issue; the surfaces use SDL's masks for big-endian.

How to build
------------
    make -f amiga/Makefile.amigaos -j2 \
         STOVE=/path/to/openamigagcc-16.2.0-open4 \
         NDK=/path/to/OpenNDK/Include_H SDL2KIT=/path/to/opengpu-sdk

The defaults are the AmigaChrome Team 2 paths. The icons are made with
amicradle's port_icons.py from amiga/icons/icons.json.

Data and licences
-----------------
All the data ships with upstream; LICENSES.txt in the drawer lists every
file's author and licence: the program is GPL 3 or later (COPYING.txt);
the graphics are GPL 2 or later (Parallel Realities), CC BY-SA 3.0,
CC BY 3.0 (Daniel Cook and others), CC0 and public domain (NASA); the
sounds and music are CC BY 3.0, CC BY-SA 3.0 and CC0 (OpenGameArt
authors). Upstream's TakaoPGothic font (IPA Font License) is only for the
SDL_ttf build and the locale/ translations only for gettext: neither is
in the drawer.

Known limits
------------
- English only, and no Unicode text (no SDL_ttf/Pango, no gettext).
- 640x480 by default; on a 68040 the game may run under its 60 frames a
  second: try Starfighter -noaudio to see how much the Ogg decoding costs.
- Not yet tried on the Amiga: built only.
