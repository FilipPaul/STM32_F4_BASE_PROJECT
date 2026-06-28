
#include "tcpserver.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

static void tcp_parse_command(char * command, struct netconn *conn){
	
	//take mutex for commandTemplate
	xSemaphoreTake(commandTemplateMutex, portMAX_DELAY);
	commandTemplate current_command;
	if (updateCommandStruct(command, &current_command, TCP_THREAD) == 0)
	{
		static const char parse_error[] = "ERROR;Command parse failed or payload too long\n";
		xSemaphoreGive(commandTemplateMutex);
		netconn_write(conn, parse_error, sizeof(parse_error) - 1U, NETCONN_COPY);
		return;
	}
	current_command.TCP_client_netconn = conn;
	//Put commandTemplate into queue to be processed by main loop
	xQueueSend(commandQueueHandle, &current_command, 0);
	//runCommand(&current_command);
	//netconn_write(conn, current_command.response, strlen(current_command.response), NETCONN_COPY);
	xSemaphoreGive(commandTemplateMutex);
}

extern struct netif gnetif;


static void tcp_byte_avaiable(uint8_t byte, struct netconn *conn)
{
	static uint16_t bfr_pointer;
	static char data[CMD_BUFFER_LENGTH];

	if(bfr_pointer < CMD_BUFFER_LENGTH && byte >= 32 && byte <=127) data[bfr_pointer++] = byte; //
	if ( (byte == '\n' || byte == '\r') && bfr_pointer > 0){
		data[bfr_pointer] = '\0'; //add null terminator -> make string from bytearray
		//printf("Received: %s\n", data);
		tcp_parse_command(data, conn);
		bfr_pointer = 0; //reset buffer pointer
	}
}

void tcp_thread(void *arg)
{
	struct netconn *conn; //conn is the listening socket,
	struct netconn *newconn; //newconn is the socket for the new connection
	err_t err, accept_err;
	static struct netbuf *buf;
	uint8_t *message_from_client;
	uint16_t message_from_client_length;
	LWIP_UNUSED_ARG(arg);
	newconn = NULL;
	conn = netconn_new(NETCONN_TCP);	

	if (conn!=NULL)
	{  
		err = netconn_bind(conn, NULL, TCP_SERVER_PORT);
		if (err == ERR_OK)
		{
			err = netconn_listen(conn);
			if (err != ERR_OK)
			{
				netconn_delete(conn);
				return;
			}
			netconn_set_nonblocking(conn, 1);
			while (1) 
			{
				if (!netif_is_link_up(&gnetif)) {
					osDelay(400);
					continue; //wait for ethernet cable to be plugged in before accepting connections
				} 
				/* Grab new connection. */
				accept_err = netconn_accept(conn, &newconn);
				if (accept_err == ERR_WOULDBLOCK)
				{
					osDelay(1);
					continue;
				}

			
				/* Process the new connection. */
				if (accept_err == ERR_OK) 
				{
					err_t recv_err;
					netconn_set_nonblocking(newconn, 1);
					//printf("IP address: %d",newconn.pcb->ip->remote_ip);
					while (1)
					{
						if (!netif_is_link_up(&gnetif))
						{
							break;
						}

						recv_err = netconn_recv(newconn, &buf);
						if (recv_err == ERR_WOULDBLOCK)
						{
							osDelay(10);
							continue;
						}

						if (recv_err != ERR_OK)
						{
							break;
						}

						do 
						{
							netbuf_data(buf, (void**)&message_from_client, &message_from_client_length);
							while(message_from_client_length--) tcp_byte_avaiable(*message_from_client++, newconn);
							//netconn_write(newconn, message_from_client, message_from_client_length, NETCONN_COPY);
						} 
						while (netbuf_next(buf) >= 0);

						netbuf_delete(buf);
						buf = NULL;
					}
					/* Close connection and discard connection identifier. */
					netconn_close(newconn);
					netconn_delete(newconn);
					newconn = NULL;
				}
			}
		}//if (err == ERR_OK)
		else
		{
			netconn_delete(conn);
		}
	}//if (conn!=NULL)
}


//void tcp_server_init(void)
//{
//	sys_thread_new("tcp_thread", tcp_thread, NULL, 2048,osPriorityAboveNormal);
//}
