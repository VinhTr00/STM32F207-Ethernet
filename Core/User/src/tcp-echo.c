/* Includes ------------------------------------------------------------------*/
#include "lwip/api.h"
#include "lwip/apps/fs.h"
#include "string.h"
#include "cmsis_os.h"
#include "tcp-echo.h"
#include "queue.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TCPECHO_THREAD_PRIO    (osPriority_t) osPriorityHigh
#define TCPECHO_PORT 7

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern QueueHandle_t xQueue1;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static void tcp_echo_thread(void *arg){

	LWIP_UNUSED_ARG(arg);
	struct netconn *conn, *newconn;
	struct netbuf *buf;
	ip_addr_t *addr;
	err_t err, accept_err;
	unsigned short port;
	char msg[100];
	char smsg[200];
	static uint8_t queueSend = 1;

	conn = netconn_new(NETCONN_TCP);
	if (conn != NULL){
		/* Bind connection to well known port number 7 */
		err = netconn_bind(conn, NULL, TCPECHO_PORT);
		if (err == ERR_OK){
			/* Tell connection to go into listening mode */
			netconn_listen(conn);
			while (1){
				/* Grab new connection */
				accept_err = netconn_accept(conn, &newconn);
				/* Process the new connection */
				if (accept_err == ERR_OK){
					while ( netconn_recv(newconn, &buf) == ERR_OK){
						/* Extract the address and port in case they are required */
						addr = netbuf_fromaddr(buf);
						port = netbuf_fromaddr(buf);
						memset(smsg, '\0', 200);
						xQueueSendToBack(xQueue1, &queueSend, 0);

						/* If there is some data remaining to be sent, the following process will continue */
						do {
							strncpy(msg, buf->p->payload, buf->p->len);
							int len = sprintf(smsg, "\n\"%s\" was sent by the the Server\n", msg);

							netconn_write(newconn, smsg, len, NETCONN_COPY);
							memset(msg, '\0', 100);
						}
						while (netbuf_next(buf) > 0);
						netbuf_delete(buf);
					}
					/* Close connection and discard connection identifier */
					netconn_close(newconn);
					netconn_delete(newconn);
				}
			}
		}
		else {
			netconn_delete(newconn);
		}
	}
}

void tcp_echo_init(void)
{
	sys_thread_new("TCPECHO", tcp_echo_thread, NULL, DEFAULT_THREAD_STACKSIZE, TCPECHO_THREAD_PRIO);
}
