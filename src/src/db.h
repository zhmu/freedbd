#include "options.h"

#ifndef __DB_H__
#define __DB_H__

#define DB_MAX_TRACKS 99

struct DB_ENTRY {
	char* discid;
	char* artist;
	char* album;
	int   year;
	int   numtracks;
	char* track[DB_MAX_TRACKS];
};

struct DB_ENTRY* db_getdisc(char* discid);
int db_load_txt(char* fname);
int db_load_xml(char* fname);
void db_dump();

extern int db_numentries;
extern struct DB_ENTRY* db;


#endif /* __DB_H__ */

/* vim:set ts=2 sw=2: */
