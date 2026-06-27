#ifndef INC_TCPSERVER_H_
#define INC_TCPSERVER_H_

#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"

#include "string.h"
#include "main.h"
#include "commandParser.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "stdio.h"
#include "globals.h"
#define TCP_SERVER_PORT 7
#define CMD_BUFFER_LENGTH 100
//void tcp_server_init(void);
void tcp_thread(void *arg);
#endif
