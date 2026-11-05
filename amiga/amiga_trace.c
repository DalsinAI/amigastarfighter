/* See amiga_trace.h.
 * The AmiCradle Team. GPL 3 or later, like Starfighter. */
#include <stdarg.h>
#include <stdio.h>
#include "amiga_trace.h"

#ifdef AMIGA_TRACE
void amiga_trace(const char *fmt, ...)
{
	static int line;
	va_list ap;
	FILE *fp = fopen("PROGDIR:sftrace.txt", line ? "a" : "w");

	if (fp == NULL)
		return;
	fprintf(fp, "%04d ", line++);
	va_start(ap, fmt);
	vfprintf(fp, fmt, ap);
	va_end(ap);
	fputc('\n', fp);
	fflush(fp);
	fclose(fp);
}
#endif
