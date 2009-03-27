#include "gcin.h"

void p_err(char *fmt,...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr,"gcin:");
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr,"\n");
	exit(-1);
}

void dbg(char *fmt,...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	fflush(stdout);
	va_end(args);
}

void tsin_db_file_name(char *tsfname)
{
}
