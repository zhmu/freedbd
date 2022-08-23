#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "db.h"
#include "server.h"
#include "util.h"

/*! \brief Shows usage instructions */
void
usage()
{
	fprintf(stderr,"usage: freedbd [-h?dbv] ");
#ifdef WITH_XML
	fprintf(stderr,"[-x database.xml] ");
#endif /* WITH_XML */
	fprintf(stderr,"[-t database.txt]\n\n");
	fprintf(stderr,"    -h, -?             this help\n");
	fprintf(stderr,"    -d                 daemonize after startup\n");
  fprintf(stderr,"    -v                 increase verbosity (more times is more verbose)\n");
	fprintf(stderr,"    -b                 dump database after loading (if twice, quit after dump)\n");
#ifdef WITH_XML
	fprintf(stderr,"    -x database.xml    XML database to load\n");
#endif /* WITH_XML */
	fprintf(stderr,"    -t database.txt    text database to load\n");
	fprintf(stderr,"\n");
}

/*! \brief The main program
 *  \param argc Argument count
 *  \param argv Arguments
 *  \return EXIT_SUCCESS on success, otherwise EXIT_FAILURE
 */
int
main(int argc,char* argv[])
{
	int ch;
	char* dbfile_xml = NULL;
	char* dbfile_txt = NULL;
	int dump_db = 0, daemonize = 0;

	gVerbosity = 0;
	while ((ch = getopt (argc, argv, "h?dbvt:x:")) != -1) {
		switch (ch) {
			case 'h':
			case '?': /* help */
			          usage();
			          return EXIT_SUCCESS;
			case 'b': /* dump database */
			          dump_db++;
			          break;
			case 'v': /* verbosity */
			          gVerbosity++;
			          break;
			case 'd': /* daemonize */
			          daemonize++;
			          break;
			case 't': /* text file */
			          dbfile_txt = strdup (optarg);
			          break;
			case 'x': /* xml file */
#ifdef WITH_XML
			          dbfile_xml = strdup (optarg);
			          break;
#else
			          fprintf(stderr, "freedbd: XML files not supported by this build\n");
			          return EXIT_FAILURE;
#endif /* !WITH_XML */
		}
	}

	if ((dbfile_txt != NULL) && (dbfile_xml != NULL)) {
		fprintf(stderr, "freedbd: can't specify both txt and xml files\n");
		return EXIT_FAILURE;
	}
	if ((dbfile_txt == NULL) && (dbfile_xml == NULL)) {
		fprintf(stderr, "freedbd: you must specify an input database, see -h for help\n");
		return EXIT_FAILURE;
	}

#ifdef WITH_XML
	if (dbfile_xml != NULL) {
		if (!db_load_xml(dbfile_xml))
			return EXIT_FAILURE;
	} else
#endif
		if (!db_load_txt(dbfile_txt))
			return EXIT_FAILURE;

	if (dump_db)
		db_dump();
	if (dump_db >= 2)
		return EXIT_SUCCESS;

	if (!server_init())
		return EXIT_FAILURE;

	if (daemonize)
		if (daemon (1, 1) < 0)
			perror("daemon");

	while (server_loop())
		;

	return EXIT_SUCCESS;
}

/* vim:set ts=2 sw=2: */
