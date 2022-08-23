/*! \file db_xml.c
 *  \brief Handle disc database from an XML file
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
#include "options.h"

#ifdef WITH_XML

#include <libxml/parser.h>
#include <libxml/tree.h>

/*! \brief Parses a CD disc
 *  \param doc Document in which the node resides
 *  \param cur Node to parse
 *  \return Zero on failure, non-zero on success
 */
static int
db_xml_parsedisc (xmlDocPtr doc, xmlNodePtr cur)
{
	struct DB_ENTRY* curentry;
	char* artist;
	char* album;
	char* discid;
	char* nr;
	char* title;
	char* tmp;
	char* ptr;
	int   tracknr;

	db = realloc (db, (++db_numentries * sizeof (struct DB_ENTRY)));
	if (db == NULL) {
		fprintf (stderr, "db_load_xml(): out of memory\n");
		return 0;
	}
	curentry = &db[db_numentries - 1];
	memset (curentry, 0, sizeof (struct DB_ENTRY));

	/* grab the id, artist and album attributes */
	discid = (char*)xmlGetProp (cur, (const xmlChar*)"id");
	artist = (char*)xmlGetProp (cur, (const xmlChar*)"artist");
	album  = (char*)xmlGetProp (cur, (const xmlChar*)"album");
	if ((discid == NULL) || (artist == NULL) || (album == NULL)) {
		fprintf(stderr, "db_load_xml(): disc without required id, artist, album attributes found, exiting\n");
		return 0;
	}
	curentry->discid = strdup (discid);
	curentry->artist = strdup (artist);
	curentry->album  = strdup (album);

	/* wade through all children */
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp (cur->name, (const xmlChar*)"year")) {
			tmp = xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp == NULL) {
				fprintf(stderr, "db_load_xml(): disc [%s] has <year> without content\n", discid);
				return 0;
			}
			curentry->year = strtol (tmp, &ptr, 10);
			if (*ptr) {
				fprintf(stderr, "db_load_xml(): disc [%s] year '%s' not parsable\n", discid, tmp);
				return 0;
			}
		}

		if (!xmlStrcmp (cur->name, (const xmlChar*)"track")) {
			nr    = (char*)xmlGetProp (cur, (const xmlChar*)"nr");
			title = (char*)xmlGetProp (cur, (const xmlChar*)"title");
			if ((nr == NULL) || (title == NULL)) {
				fprintf(stderr, "db_load_xml(): disc [%s] has track without required nr,title attributes, exiting\n", discid);
				return 0;
			}
			tracknr = strtol (nr, &ptr, 10);
			if (*ptr) {
				fprintf(stderr, "db_load_xml(): disc [%s] track '%s' not parsable\n", discid, nr);
				return 0;
			}
			if ((tracknr < 1) || (tracknr > DB_MAX_TRACKS)) {
				fprintf (stderr, "db_load_xml(); track# %d out of range\n", tracknr);
				return 0;
			}
			if (tracknr > 1)
				if (curentry->track[tracknr - 2] == NULL) {
					fprintf(stderr, "db_load_xml(): track# %d not specified for disc %s\n", tracknr - 1, curentry->discid);
					return 0;
				}
			curentry->track[tracknr - 1] = strdup (title);
			curentry->numtracks = tracknr;
		}

		cur = cur->next;
	}

	return 1;
}

/*! \brief Reads the database from disk
 *  \param fname File to read
 *  \return Zero on failure, non-zero on success
 */
int
db_load_xml(char* fname)
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	/* do not generate nodes for formatting space; this is annoying */
	LIBXML_TEST_VERSION
	xmlKeepBlanksDefault (0);

	doc = xmlParseFile (fname);
	if (doc == NULL)
		return 0;

	cur = xmlDocGetRootElement (doc);
	if (cur == NULL) {
		fprintf(stderr, "db_load_xml(): file is empty\n");
		return 0;
	}
	if (xmlStrcmp (cur->name, (const xmlChar*)"discs")) {
		fprintf(stderr, "db_load_xml(): invalid type, root node != discs\n");
		return 0;
	}
	cur = cur->xmlChildrenNode;
	while (cur && xmlIsBlankNode (cur)) cur = cur->next;

	while (cur != NULL) {
		if (!xmlStrcmp (cur->name, (const xmlChar*)"disc"))
			if (!db_xml_parsedisc (doc, cur))
				return 0;

		cur = cur->next;
	}

	return 1;
}

#endif /* WITH_XML */

/* vim:set ts=2 sw=2: */
