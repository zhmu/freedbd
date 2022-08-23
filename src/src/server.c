/*! \file server.c
 *  \brief Handles server-ish stuff
 */
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "conn.h"
#include "server.h"
#include "util.h"

int server_socket = -1;
struct SERVER_CONN conn[MAX_CONNS];

/*! \brief Allocates a new connection slot
 *  \param fd File descriptor to use
 *  \return The connection slot on success, NULL on failure
 */
static struct SERVER_CONN*
server_alloc_conn (int fd)
{
	int i;

	for (i = 0; i < MAX_CONNS; i++)
		if (conn[i].fd == FD_NONE) {
			conn[i].fd = fd;
			return &conn[i];
		}

	return NULL;
}


/*! \brief Initializes the server socket
 *  \return Zero on failure, non-zero on success
 */
int
server_init()
{
	struct sockaddr_in sin;
	int i;

	/* no connections just yet */
	for (i = 0; i < MAX_CONNS; i++) {
		conn[i].fd = FD_NONE;
		conn[i].f = NULL;
	}

	server_socket = socket (AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("socket");
		return 0;
	}

	memset (&sin, 0, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons (PORT_NO);
	if (bind (server_socket, (struct sockaddr*)&sin, sizeof (sin)) < 0) {
		perror("bind");
		return 0;
	}

	if (listen (server_socket, 5) < 0) {
		perror("listen");
		return 0;
	}

	return 1;
}

/*! \brief Handles the server loop
 *  \return Zero on failure, non-zero on success
 */
int
server_loop()
{
	fd_set fds;
	int i, fd, nsocks = server_socket, len, slotnr;
	struct sockaddr sa;
	socklen_t sl;
	char line[MAX_COMMAND_LEN], ch;
	struct SERVER_CONN* slot;
	char* ptr;

	FD_ZERO(&fds);
	FD_SET(server_socket, &fds);
	for (i = 0; i < MAX_CONNS; i++)
		if (conn[i].fd != FD_NONE) {
			FD_SET(conn[i].fd, &fds);
			nsocks++;
		}

	if (select (nsocks + 1, &fds, NULL, NULL, (struct timeval*)NULL) < 0) {
		perror("select");
		return 0;
	}

	if (FD_ISSET (server_socket, &fds)) {
		/* new connection */
		sl = sizeof (sa);
		fd = accept (server_socket, &sa, &sl);
		if (fd < 0) {
			perror("accept");
		} else {
			slot = server_alloc_conn (fd);
			if (slot == NULL) {
				/* out of connections! */
				fprintf(stderr, "warning: out of connections, dropping one\n");
				conn_drop (fd);
				close (fd);
			} else {
				VPRINTF(3,"[%p] accepted connection: fd=%u\n", slot, fd);
				slot->f = fdopen (fd, "a+t");
				if (slot->f == NULL) {
					perror ("fdopen");
				}
				conn_hello (slot);
			}
		}
	}

	for (slotnr = 0; slotnr < MAX_CONNS; slotnr++) {
		slot = &conn[slotnr];
		if (slot->fd == FD_NONE)
			continue;

		if (FD_ISSET (slot->fd, &fds)) {
			/* incoming data here. try to read it */
			len = 0;
			while ((i = read (slot->fd, &ch, 1) > 0)) {
				if (ch == '\n') break;
				line[len] = ch;
				if (len >= MAX_COMMAND_LEN - 1) break;
				len++;
			}
			if (len == 0) {
				/* no data. connection probably died */
				VPRINTF(3,"[%p] dropped connection\n", slot);
				fclose (slot->f); slot->fd = FD_NONE;
			} else {
				line[len] = 0;
			  while ((ptr = strchr (line, '\r')) != NULL) *ptr = 0;
				conn_handle (slot, line);
			}
		}
	}

	return 1;
}


/* vim:set ts=2 sw=2: */
