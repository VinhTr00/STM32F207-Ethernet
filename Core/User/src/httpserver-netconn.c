/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/httpser-netconn.c 
  * @author  MCD Application Team
  * @brief   Basic http server implementation using LwIP netconn API  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "lwip/api.h"
#include "lwip/apps/fs.h"
#include "string.h"
#include "httpserver-netconn.h"
#include "cmsis_os.h"

#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define WEBSERVER_THREAD_PRIO    (osPriority_t) osPriorityNormal
#define WEBSERVER_PORT 80

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osSemaphoreId_t xSemaphoreHTTP = NULL;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

static void http_server_serve(struct netconn *conn)
{
	struct netbuf *inbuf;
	err_t recv_err;
	char* buf;
	u16_t buflen;
	struct fs_file file;

	/* Read the data from the port, blocking if nothing yet there. */
	recv_err = netconn_recv(conn, &inbuf);
	osSemaphoreRelease(xSemaphoreHTTP);
	if (recv_err == ERR_OK)
	{
		if (netconn_err(conn) == ERR_OK)
		{
			netbuf_data(inbuf, (void**)&buf, &buflen);
			/* Is this an HTTP GET command? (only check the first 5 chars, since
			  there are other formats for GET, and we're keeping it very simple )*/
			/* Check if request to get ST.gif */
			if (strncmp((char const *)buf,"GET /STM32F2x7_files/ST.gif",27)==0)
			{
				fs_open(&file, "/STM32F2x7_files/ST.gif");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			/* Check if request to get stm32.jpeg */
			else if (strncmp((char const *)buf,"GET /STM32F2x7_files/stm32.jpeg",31)==0)
			{
				fs_open(&file, "/STM32F2x7_files/stm32.jpeg");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if (strncmp((char const *)buf,"GET /STM32F2x7_files/logo.jpg", 29) == 0)
			{
				/* Check if request to get ST logo.jpg */
				fs_open(&file, "/STM32F2x7_files/logo.jpg");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if((strncmp(buf, "GET /index.html", 19) == 0)||(strncmp(buf, "GET / ", 6) == 0))
			{
				/* Load STM32F2xx page */
				fs_open(&file, "/index.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else
			{
				/* Load Error page */
				fs_open(&file, "/404.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
		}
	}
	/* Close the connection (server closes in HTTP) */
	netconn_close(conn);
	/* Delete the buffer (netconn_recv gives us ownership,
	so we have to make sure to deallocate the buffer) */
	netbuf_delete(inbuf);
}

/**
  * @brief  http server thread 
  * @param arg: pointer on argument(not used here) 
  * @retval None
  */
static void http_server_netconn_thread(void *arg)
{
  LWIP_UNUSED_ARG(arg);
  struct netconn *conn, *newconn;
  err_t err, accept_err;

  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);

  if (conn!= NULL)
  {
	  /* Bind to port 80 (HTTP) with default IP address */
	err = netconn_bind(conn, NULL, WEBSERVER_PORT);
    if (err == ERR_OK)
    {
      /* Put the connection into LISTEN state */
      netconn_listen(conn);

      while(1)
      {
        /* accept any icoming connection */
        accept_err = netconn_accept(conn, &newconn);
        if(accept_err == ERR_OK)
        {
          /* serve connection */
          http_server_serve(newconn);
          /* delete connection */
          netconn_delete(newconn);
        }
      }
    }
  }
}

/**
  * @brief  Initialize the HTTP server (start its thread) 
  * @param  none
  * @retval None
  */
void http_server_netconn_init()
{
	sys_thread_new("HTTP", http_server_netconn_thread, NULL, DEFAULT_THREAD_STACKSIZE, WEBSERVER_THREAD_PRIO);
	xSemaphoreHTTP = osSemaphoreNew(1, 1, NULL);
}

