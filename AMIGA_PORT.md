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


Standard keys
-------------

Every AmiCradle game has the same keys on Right Amiga (GFOpenUp's
SDL2.module, after 0.8.26):

- Right Amiga + F: window or full screen, remembered for next time
- Right Amiga + P: pause (Right Amiga + P again goes on)
- Right Amiga + Q: quit
- Right Amiga + M: free the mouse pointer, or let the game take it again
- Right Amiga + S: a screenshot in the drawer's Screenshots
- Help: this game's keys (the drawer's Controls file)
- Right Amiga + F1, F2: F11, F12; Right Amiga + cursor keys: Page Up,
  Page Down, Home, End

The FULLSCREEN tooltype on the game's icon (take the brackets off) starts
it full screen.


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
- amiga/tools/png_rgba.py (run when the drawer is made): the 41 8-bit RGB
  PNGs in gfx/ go into the drawer as RGBA, the same pixels, opaque. See
  "The emulator crash" below. Those files differ from upstream's only in
  how they are stored (their licences in LICENSES.txt still apply).
- make TRACE=1: the program notes each step of its SDL set-up (window,
  renderer, texture, every IMG_Load with the size and format it got, the
  first frames) in PROGDIR:sftrace.txt, one open/write/close per line, so
  the file is complete even when the machine goes down (amiga/amiga_trace.h).

The emulator crash (SDL2_image.module 0.6)
------------------------------------------
The first build took the AmigaChrome emulator itself down within seconds
of starting (a1200native: glibc "corrupted size vs. prev_size", "machine
exited (-6)"), with or without -noaudio, before the title.

The trace showed it inside IMG_Load("gfx/missileBoat1.png"), the first
sprite of a group loaded at start-up. The trigger is SDL2_image.module
decoding (on the host) 8-bit RGB PNGs (colour type 2) whose rows are not a
multiple of 4 bytes: missileBoat1.png is 30x31, 90 bytes a row. The module
hands back a good SDL_PIXELFORMAT_RGB888 surface (pitch 120, the right
pixels), but the host's heap is damaged: with two of them loaded and kept,
the third IMG_Load of the same file aborts the emulator. Loaded and freed
one at a time, the same file loads 8 times without trouble; nothing else is
needed (no SDL_Init, no window, no conversion).

What was tried (amiga/tests, each step logged to a file on Work:):
- rgb_filter.c (RGBFilter): synthetic PNGs from mkpng.py, kept.
  30x31 RGB, all rows Paeth: crash at the 3rd load. 30x31 RGB, all rows
  None, Sub, Up or Average: clean. 32x31 RGB (96-byte rows), Paeth: clean.
  RGBFilter keep file gfx/missileBoat1.png (rows Sub, Average and Paeth):
  crash at the 3rd load; the same file stored again without Paeth rows
  (Sub, Up, Average): crash too; the same pixels as RGBA (30x31, 120-byte
  rows): 16 kept loads, clean. So it depends on the data, not only on the
  filter, but every crash so far was an RGB PNG with rows not a multiple
  of 4, and no RGBA or palette picture has done it.
- loadseq.c (LoadSeq): the game's own loading order (window, renderer,
  every sprite, conversions), driven from a list; how the trigger was cut
  down from the game.

For the emulator team: RGBFilter keep file gfx/missileBoat1.png (from
Work:Starfighter, the upstream file) or RGBFilter keep 30 4 (no data file
needed) brought a1200native down in every run here.

The port's workaround is in the data: png_rgba.py stores every RGB PNG as
RGBA. The game converts each sprite to its 32-bit screen format and keys
out black either way; the sprites that were RGB now come with an alpha
channel (all 255) and so are blitted with blending, the same pixels.

How to build
------------
    make -f amiga/Makefile.amigaos -j2 \
         STOVE=/path/to/openamigagcc-16.2.0-open4 \
         NDK=/path/to/OpenNDK/Include_H SDL2KIT=/path/to/opengpu-sdk

The defaults are the AmiCradle Team 2 paths. The icons are made with
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
- After a window quit, the Workbench screen's title bar stays blank where
  the game's window was, until something redraws it (seen in the lab).
- Tried in the AmigaChrome lab (OpenGPU 0.6): the title, a new game, the
  first mission (flying and firing), Pause+Escape back to the title, and
  Quit to the Shell with the settings saved. Not checked: the sound and
  music, a joystick, a full screen, the shop and saving a game.
