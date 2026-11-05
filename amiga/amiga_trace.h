/* make TRACE=1: the game notes each step of its SDL set-up in
 * PROGDIR:sftrace.txt, opened, written and closed per line, so what it got
 * to is on the disk even when the machine (or the emulator) goes down.
 * The AmiCradle Team. GPL 3 or later, like Starfighter. */
#ifndef AMIGA_TRACE_H
#define AMIGA_TRACE_H
#ifdef AMIGA_TRACE
void amiga_trace(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#define ATRACE(...) amiga_trace(__VA_ARGS__)
#else
#define ATRACE(...) ((void)0)
#endif
#endif
