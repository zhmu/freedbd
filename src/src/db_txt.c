/*! \file db_txt.c
 *  \brief Handle disc database from an INI-like text file
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

/*! \brief Reads the database from disk
 *  \param fname File to read
 *  \return Zero on failure, non-zero on success
 */
int
db_load_txt(char* fname)
{
	char line[MAX_COMMAND_LEN];	
	char* ptr;
	char* key;
	char* val;
	struct DB_ENTRY* curentry = NULL;
	int tracknr;

	FILE* f = fopen(fname, "rt");
	if (f == NULL) {
		perror("fopen");
		return 0;
	}

	while (fgets (line, sizeof (line), f)) {
		while ((ptr = strchr (line, '\n')) != NULL) *ptr = 0;
	
		if (!strlen (line))
			continue;

		/* disc id? */
		if ((line[0] == '[') && (line[strlen (line) - 1] == ']')) {
			/* yes. isolate it */
			line[strlen (line) - 1] = 0;

			db = realloc (db, (++db_numentries * sizeof (struct DB_ENTRY)));
			if (db == NULL) {
				fprintf (stderr, "db_load_txt(): out of memory\n");
				goto fail;
			}
			curentry = &db[db_numentries - 1];
			memset (curentry, 0, sizeof (struct DB_ENTRY));

			curentry->discid = strdup (line + 1);
			continue;
		}

		/* ignore lines which begin with # or ;, these are comments */
		if ((line[0] == ';') || (line[0] == '#'))
			continue;

		/* isolate key=val pairs */
		key = line; val = strchr (line, '=');
		if (val == NULL) {
			fprintf (stderr, "db_load_txt(): file is corrupt, line '%s'\n", line);
			goto fail;
		}
		*val++ = 0;

		while (isspace (key[strlen (key) - 1])) key[strlen (key) - 1] = 0;
		while (isspace (val[strlen (val) - 1])) val[strlen (val) - 1] = 0;

		if (curentry == NULL) {
			fprintf (stderr, "db_load_txt(): key '%s' without disc id\n", key);
			goto fail;
		}

		tracknr = strtol (key, &ptr, 10);
		if (!*ptr) {
			/* track# */
			if ((tracknr < 1) || (tracknr > DB_MAX_TRACKS)) {
				fprintf (stderr, "db_load_txt(); track# %d out of range\n", tracknr);
				fclose(f);
				return 0;
			}
			if (tracknr > 1)
				if (curentry->track[tracknr - 2] == NULL) {
					fprintf(stderr, "db_load_txt(): track# %d not specified for disc %s\n", tracknr - 1, curentry->discid);
					goto fail;
				}
			curentry->track[tracknr - 1] = strdup (val);
			curentry->numtracks = tracknr;
			continue;
		}

		if (!strcasecmp (key, "artist")) {
			curentry->artist = strdup (val);
			continue;
		}

		if (!strcasecmp (key, "album")) {
			curentry->album = strdup (val);
			continue;
		}

		if (!strcasecmp (key, "year")) {
			curentry->year = strtol (val, &ptr, 10);
			if (*ptr) {
				fprintf (stderr, "db_load_txt(): year [%s] is malformed\n", val);
				goto fail;
			}
			continue;
		}
	}

	fclose(f);
	return 1;

fail:
	fclose(f);
	if (db) free(db); db = NULL;
	return 0;
}

/* vim:set ts=2 sw=2: */
