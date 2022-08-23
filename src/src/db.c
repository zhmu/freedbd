/*! \file db.c
 *  \brief Handle disc database from a text file
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "db.h"

int db_numentries = 0;
struct DB_ENTRY* db = NULL;


/*! \brief Returns a disk by Disc ID
 *  \param discid Disk ID to query 
 *  \return Entry on success, NULL on failure
 */
struct DB_ENTRY*
db_getdisc(char* discid)
{
	struct DB_ENTRY* curentry = db;
	int i;

	for (i = 0; i < db_numentries; i++, curentry++)
		if (!strcasecmp (curentry->discid, discid))
			return curentry;

	return NULL;
}

/*! \brief Dumps the database to the screen */
void
db_dump()
{
	struct DB_ENTRY* curentry = db;
	int i, nr;

	for (i = 0; i < db_numentries; i++, curentry++) {
		printf ("Disc ID: <%s>: %s / %s (%u)\n", curentry->discid, curentry->artist, curentry->album, curentry->year);
		for (nr = 0; nr < curentry->numtracks; nr++)
			printf (" %2u - %s\n", nr + 1, curentry->track[nr]);
		printf ("\n");
	}
}

/* vim:set ts=2 sw=2: */
