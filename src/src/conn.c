/*! \file conn.c
 *  \brief Handle individual connections
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "conn.h"
#include "db.h"
#include "util.h"

/*! \brief Sends formatted output to a connection
 *  \param slot Connection slot
 *  \param fmt The format used
 */
static void
conn_sendf (struct SERVER_CONN* slot, char* fmt, ...)
{
	va_list va;

	va_start (va, fmt);
	vfprintf (slot->f, fmt, va);
	va_end (va);

	fflush (slot->f);

	if (gVerbosity >= 2) {
		printf ("[%p] >>> ", slot);
		va_start (va, fmt);
		vprintf (fmt, va);
		va_end (va);
	}
}

/*! \brief Says hello to new connections
 *  \param fd File descriptor to greet
 */
void
conn_hello (struct SERVER_CONN* slot)
{
	conn_sendf (slot, "200 ready to serve\n");
}

static int
conn_cmd_hello (struct SERVER_CONN* slot, char* data)
{
	conn_sendf (slot, "200 hello\n");
	return 1;
}

static int
conn_cmd_proto (struct SERVER_CONN* slot, char* data)
{
	conn_sendf (slot, "200 whatever\n");
	return 1;
}

static int
conn_cmd_query (struct SERVER_CONN* slot, char* data)
{
	char* discid = (data + 6);
	char* ptr;
	int numtracks;
	struct DB_ENTRY* db;

	/* query [discid] [numtracks] [lentrack0] .. [lentrackn] [disclen] */
	ptr = strchr (discid, ' ');
	if (ptr == NULL) {
		conn_sendf (slot, "500 wtf?\n");
		return 0;
	}
	*ptr++ = 0;
	numtracks = strtol (ptr, &ptr, 10);
	if (*ptr != ' ') {
		conn_sendf (slot, "500 wtf?\n");
		return 0;
	}

	VPRINTF(1,"query for discid [%s], %d track(s)\n", discid, numtracks);
	db = db_getdisc (discid);
	if (db == NULL)
		conn_sendf (slot, "202\n");
	else
		conn_sendf (slot, "200 cd %s\n", discid);

	/* send 200 <string> <md> for one match */
	/* send 211 <string> <md> <string> <md> ... for >1 match */
	/* send 202 for no matches */
	/*conn_sendf (slot, "202 rot op\n");*/

	return 0;
}

static int
conn_cmd_read (struct SERVER_CONN* slot, char* data)
{
	int i;
	char* ptr;
	char* discid;
	struct DB_ENTRY* db;

	/* read blaat <id> */
	ptr = strchr (data, ' ');
	if (ptr == NULL) {
		conn_sendf (slot, "501 wtf?\n");
		return 0;
	}
	ptr = strchr (ptr + 1, ' ');
	if (ptr == NULL) {
		conn_sendf (slot, "502 wtf?\n");
		return 0;
	}
	ptr++; discid = ptr;

	db = db_getdisc (discid);
	if (db == NULL) {
		conn_sendf (slot, "500 wtf?\n");
		return 0;
	}

	conn_sendf (slot, "210 ok\n");
	if (db->year)
		conn_sendf (slot, "DYEAR=%u\n", db->year);
	if (db->album)
		conn_sendf (slot, "TITLE=%s\n", db->album);
	if (db->artist)
		conn_sendf (slot, "ARTIST=%s\n", db->artist);
	if (db->album && db->artist)
	conn_sendf (slot, "DTITLE=%s / %s\n", db->artist, db->album);
	for (i = 0; i < db->numtracks; i++)
		conn_sendf (slot, "TTITLE%u=%s\n", i, db->track[i]);
	/* disc length is needed for p5-CDDB. we don't know it, so use a dummy
   * value */
	conn_sendf (slot, "DISC LENGTH=42\n");
	conn_sendf (slot, ".\n");
	return 1;
}

static int
conn_cmd_quit (struct SERVER_CONN* slot, char* data)
{
	conn_sendf (slot, "200 bye\n");
	shutdown (slot->fd, SHUT_RDWR);
	close (slot->fd);
	fclose (slot->f);
	slot->fd = FD_NONE;
	VPRINTF(3,"[%p] closed connection\n", slot);
	return 1;
}

/*! \brief Tells a connection we drop it
 *  \param fd File descriptor to drop
 */
void
conn_drop(int fd)
{
	write (fd, "423 Out of connections, sorry\n", 30);
}

/*! \brief Handles incoming data
 *  \param fd File descriptor
 *  \param data Data received
 *  \return Zero on failure, non-zero on success
 */
int
conn_handle (struct SERVER_CONN* slot, char* data)
{
	VPRINTF(2,"[%p] <<< %s\n", slot, data);
	if (!strncasecmp (data, "quit",   4)) return conn_cmd_quit  (slot, data);
	if (strncasecmp (data, "cddb ", 5)) {
		conn_sendf (slot, "500 huh?\n");
		return 0;
	}
	data += 5;

	if (!strncasecmp (data, "hello ", 6)) return conn_cmd_hello (slot, data);
	if (!strncasecmp (data, "proto ", 6)) return conn_cmd_proto (slot, data);
	if (!strncasecmp (data, "query ", 6)) return conn_cmd_query (slot, data);
	if (!strncasecmp (data, "read ",  5)) return conn_cmd_read  (slot, data);

	conn_sendf (slot, "500 huh?\n");
	return 0;
}

/* vim:set ts=2 sw=2: */
