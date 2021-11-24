#include "../../../Library/console.h"

#include "windows_socket.h"

#include <stdio.h>
/* Standard includes. */
#include <stdio.h>
#include <time.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_TCP_server.h"
#include "FreeRTOS_DHCP.h"

/* FreeRTOS+FAT includes. */
#include "ff_headers.h"
#include "ff_stdio.h"
#include "ff_ramdisk.h"

/* Demo application includes. */
#include "SimpleUDPClientAndServer.h"
#include "TwoUDPEchoClients.h"
#include "TCPEchoClient_SingleTasks.h"
#include "TCPEchoClient_SeparateTasks.h"
#include "UDPCommandConsole.h"
#include "TCPCommandConsole.h"
#include "UDPSelectServer.h"
#include "SimpleTCPEchoServer.h"
#include "TFTPServer.h"
#include "demo_logging.h"




SOCKET open_tcp_connection()
{
    struct freertos_sockaddr  pxAddress;

    Socket_t xClientSocket;
    socklen_t xSize = sizeof(pxAddress);
    static const TickType_t xTimeOut = pdMS_TO_TICKS(2000);

    /* Attempt to open the socket. */
    xClientSocket = FreeRTOS_socket(PF_INET,
        SOCK_STREAM,  /* SOCK_STREAM for TCP. */
        IPPROTO_TCP);

    /* Check the socket was created. */
    configASSERT(xClientSocket != FREERTOS_INVALID_SOCKET);

    if (xClientSocket != FREERTOS_INVALID_SOCKET)
        log("socket client is created");
    else
        log("socket client is not created");
    /* If FREERTOS_SO_RCVBUF or FREERTOS_SO_SNDBUF are to be used with
    FreeRTOS_setsockopt() to change the buffer sizes from their default then do
    it here!.  (see the FreeRTOS_setsockopt() documentation. */

    /* If ipconfigUSE_TCP_WIN is set to 1 and FREERTOS_SO_WIN_PROPERTIES is to
    be used with FreeRTOS_setsockopt() to change the sliding window size from
    its default then do it here! (see the FreeRTOS_setsockopt()
    documentation. */

    /* Set send and receive time outs. */
    FreeRTOS_setsockopt(xClientSocket,
        0,
        FREERTOS_SO_RCVTIMEO,
        &xTimeOut,
        sizeof(xTimeOut));

    FreeRTOS_setsockopt(xClientSocket,
        0,
        FREERTOS_SO_SNDTIMEO,
        &xTimeOut,
        sizeof(xTimeOut));

    /* Bind the socket, but pass in NULL to let FreeRTOS+TCP choose the port number.
    See the next source code snipped for an example of how to bind to a specific
    port number. */
   
    pxAddress.sin_port = FreeRTOS_htons(15000);

    FreeRTOS_bind(xClientSocket, pxAddress.sin_port, xSize);
   
}

void send_data_to_server(
	SOCKET endpoint,
	frame_size_t frame_size,
	long* buffer_to_transmit
) {

    Socket_t xSocket;
    struct freertos_sockaddr xRemoteAddress;
    BaseType_t xAlreadyTransmitted = 0, xBytesSent = 0;
    TaskHandle_t xRxTask = NULL;
    size_t xLenToSend;
    size_t i;
    const size_t data_size =
        frame_size.header * sizeof(long)
        + frame_size.bitmap * sizeof(long);
    const unsigned short data_size_size = sizeof(data_size);
    const size_t total_length_to_send = data_size_size + data_size;
    unsigned char* send_buffer = malloc(total_length_to_send);
    log("the total length to send is %u bytes", data_size);

    for (i = 0; i < data_size_size; ++i)
        send_buffer[i] = ((unsigned char*)&data_size)[i];

    for (; (i - data_size_size) < data_size; ++i)
        send_buffer[i] = ((unsigned char*)buffer_to_transmit)[i - data_size_size];

    size_t length_to_send;
    /* Set the IP address (192.168.0.50) and port (1500) of the remote socket
    to which this client socket will transmit. */
    xRemoteAddress.sin_port = FreeRTOS_htons(15078);
    xRemoteAddress.sin_addr = FreeRTOS_inet_addr_quick(122, 168, 25, 1);

    /* Create a socket. */
    xSocket = FreeRTOS_socket(FREERTOS_AF_INET,
        FREERTOS_SOCK_STREAM,/* FREERTOS_SOCK_STREAM for TCP. */
        FREERTOS_IPPROTO_TCP);
    configASSERT(xSocket != FREERTOS_INVALID_SOCKET);

    /* Connect to the remote socket.  The socket has not previously been bound to
    a local port number so will get automatically bound to a local port inside
    the FreeRTOS_connect() function. */
    if (FreeRTOS_connect(xSocket, &xRemoteAddress, sizeof(xRemoteAddress)) == 0)
    {
        /* Keep sending until the entire buffer has been sent. */
        while (xAlreadyTransmitted < data_size)
        {
            /* How many bytes are left to send? */
            xLenToSend = data_size - xAlreadyTransmitted;
            xBytesSent = FreeRTOS_send( /* The socket being sent to. */
                xSocket,
                /* The data being sent. */
                &(((char*)send_buffer)[xAlreadyTransmitted]),
                /* The remaining length of data to send. */
                xLenToSend,
                /* ulFlags. */
                0);

            if (xBytesSent >= 0)
            {
                /* Data was sent successfully. */
                xAlreadyTransmitted += xBytesSent;
            }
            else
            {
                /* Error – break out of the loop for graceful socket close. */
                break;
            }
        }
    }

    /* Initiate graceful shutdown. */
    FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_RDWR);

    /* Wait for the socket to disconnect gracefully (indicated by FreeRTOS_recv()
    returning a FREERTOS_EINVAL error) before closing the socket. */
    while (FreeRTOS_recv(xSocket, ((char*)send_buffer), data_size, 0) >= 0)
    {
        /* Wait for shutdown to complete.  If a receive block time is used then
        this delay will not be necessary as FreeRTOS_recv() will place the RTOS task
        into the Blocked state anyway. */
        vTaskDelay(2500);

        /* Note – real applications should implement a timeout here, not just
        loop forever. */
    }

    /* The socket has shut down and is safe to close. */
    FreeRTOS_closesocket(xSocket);
    
	free(send_buffer);
}


