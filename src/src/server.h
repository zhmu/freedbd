#ifndef __SERVER_H__
#define __SERVER_H__

#define FD_NONE (-1)

struct SERVER_CONN {
	int fd;
	FILE* f;
};

int server_init();
int server_loop();

#endif /* __SERVER_H__ */

/* vim:set ts=2 sw=2: */
