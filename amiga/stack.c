/* A Shell gives 4 KB of stack; Starfighter keeps PATH_MAX buffers and
 * long strings on it. libnix's startup sets this much up instead.
 * The AmiCradle Team. GPL 3 or later, like Starfighter. */
unsigned long __stack = 256 * 1024;
static const char version[] __attribute__((used)) = "$VER: Starfighter 2.4 Amiga (05.11.2026)";
