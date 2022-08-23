#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "db.h"
#include "server.h"

/*! \brief Overal verbosity level */
int gVerbosity;

/*! \brief Verbosity printf
 *  \param level Verbosity level required to print
 *  \param fmt printf()-alike format specifier
 */
void
VPRINTF(int level, char* fmt, ...)
{
	va_list va;

	if (gVerbosity < level)
		return;

	va_start (va, fmt);
	vprintf (fmt, va);
	va_end (va);
}

/* vim:set ts=2 sw=2: */
