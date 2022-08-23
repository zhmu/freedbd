#include "server.h"

#ifndef __CONN_H__
#define __CONN_H__

void conn_drop(int fd);
void conn_hello(struct SERVER_CONN* slot);
int conn_handle (struct SERVER_CONN* slot, char* data);

#endif /* __CONN_H__ */

/* vim:set ts=2 sw=2: */
