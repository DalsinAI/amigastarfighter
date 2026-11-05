/* No gettext on AmigaOS 3.x: the game shows its English text as written.
 * (Without SDL_ttf and Pango, upstream's NOFONT build draws only the
 * bitmap font's Latin letters anyway.)
 * The AmiCradle Team. GPL 3 or later, like Starfighter. */
#ifndef AMIGA_LIBINTL_H
#define AMIGA_LIBINTL_H
#define gettext(s) (s)
#define ngettext(s, p, n) ((n) == 1 ? (s) : (p))
#define bindtextdomain(d, dir) ((const char *)(d))
#define textdomain(d) ((const char *)(d))
#endif
