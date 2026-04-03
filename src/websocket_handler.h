#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include "config.h"

void ws_init();
void ws_process();
void ws_broadcast_serial(const char *msg);

#endif