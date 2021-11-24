#include "../../../Library/console.h"

#include "windows_socket.h"
#include "data_transmission_task.h"
//#include "FreeRTOS_IP.h"
//#include "FreeRTOS_Sockets.h"

typedef struct task_init_data
{
	QueueHandle_t input_queue;
	frame_size_t frame_size;
	SOCKET socket;
} task_init_data_t;

task_init_data_t task_init_data;

//static Socket_t open_tcp_connection(const char* host, unsigned short port)
//{
//	Socket_t endpoint;
//	struct freertos_sockaddr bind_address;
//	TickType_t timeout = pdMS_TO_TICKS(2000);
//
//	endpoint = FreeRTOS_socket(
//		FREERTOS_AF_INET,
//		FREERTOS_SOCK_STREAM,
//		FREERTOS_IPPROTO_TCP
//	);
//
//	if (endpoint == FREERTOS_INVALID_SOCKET) {
//		log("data_transmission_task failed to open a TCP connection");
//		return NULL;
//	}
//
//	FreeRTOS_setsockopt(
//		endpoint,
//		0,
//		FREERTOS_SO_RCVTIMEO,
//		&timeout,
//		sizeof(timeout)
//	);
//
//	FreeRTOS_setsockopt(
//		endpoint,
//		0,
//		FREERTOS_SO_SNDTIMEO,
//		&timeout,
//		sizeof(timeout)
//	);
//
//	bind_address.sin_addr = FreeRTOS_inet_addr(host);
//	bind_address.sin_port = FreeRTOS_htons((uint16_t)port);
//
//	if (FreeRTOS_connect(endpoint, &bind_address, sizeof(bind_address)) != 0)
//		return NULL;
//}

//static void send_data_to_server(
//	SOCKET endpoint,
//	frame_size_t frame_size,
//	long* buffer_to_transmit
//) {
//	size_t i;
//	const size_t data_size =
//		frame_size.header * sizeof(long)
//		+ frame_size.bitmap * sizeof(long);
//	const unsigned short data_size_size = sizeof(data_size);
//	const size_t total_length_to_send = data_size_size + data_size;
//	unsigned char* send_buffer = malloc(total_length_to_send);
//
//	for (i = 0; i < data_size_size; ++i)
//		send_buffer[i] = ((unsigned char*)&data_size)[i];
//
//	for (; (i - data_size_size) < data_size; ++i)
//		send_buffer[i] = ((unsigned char*)buffer_to_transmit)[i - data_size_size];
//
//	BaseType_t
//		already_transmitted = 0,
//		bytes_sent = 0;
//
//	size_t length_to_send;
//
//	while (already_transmitted < total_length_to_send)
//	{
//		length_to_send = total_length_to_send - already_transmitted;
//		
//		bytes_sent = FreeRTOS_send(
//			endpoint,
//			&(buffer_to_transmit[already_transmitted]),
//			length_to_send,
//			0
//		);
//
//		if (bytes_sent == SOCKET_ERROR) {
//			log("send failed with error: %d\n", WSAGetLastError());
//			closesocket(endpoint);
//			WSACleanup();
//			return 1;
//		}
//
//		if (bytes_sent >= 0)
//			already_transmitted = already_transmitted + bytes_sent;
//		else
//			break;
//	}
//
//	free(send_buffer);
//}

//static void close_tcp_connection(SOCKET endpoint)
//{
//	unsigned char dummy_buffer;
//	
//	FreeRTOS_shutdown(endpoint, FREERTOS_SHUT_RDWR);
//
//	while (FreeRTOS_recv(endpoint, (unsigned char*)&dummy_buffer, sizeof(char), 0) >= 0)
//		vTaskDelay(250);
//
//	FreeRTOS_closesocket(endpoint);
//}

static void data_transmission_task(void* pv_parameters)
{
	log("executed data_transmission_task(0x%x)", (unsigned int)pv_parameters);
	long* encrypted_data;

	task_init_data_t* init_data = (task_init_data_t*)pv_parameters;

	while (1) {
		TickType_t current_tick_count = xTaskGetTickCount();

		encrypted_data = NULL;

		unsigned short encrypted_data_was_read = 0;
		unsigned short empty_queue = 0;

		do {
			encrypted_data_was_read = xQueueReceive(
				init_data->input_queue,
				(void*)&encrypted_data,
				(TickType_t)0
			);

			if (encrypted_data_was_read) {
				log("data_transmission_task has read a frame from the transmission queue");
				send_data_to_server(init_data->socket, init_data->frame_size, encrypted_data);
			}
			else if (!empty_queue) {
				empty_queue = 1;
				//log("the transmission queue is now empty");
			}
		} while (!encrypted_data_was_read);

		//log("data_transmission_task took %lu ticks to execute", xTaskGetTickCount() - current_tick_count);
	}

	close_tcp_connection(init_data->socket);
}

void start_data_transmission_task(
	TaskHandle_t* task_handle,
	QueueHandle_t input_queue,
	frame_size_t frame_size
	
) {
	log("executed start_data_transmission_task(...)");

	task_init_data.input_queue = input_queue;
	task_init_data.frame_size = frame_size;

	task_init_data.socket = open_tcp_connection();

	if (task_init_data.socket != INVALID_SOCKET) {
		log("connected to the server");
		xTaskCreate(
			data_transmission_task,
			"data_transmission_task",
			configMINIMAL_STACK_SIZE,
			(void*)&task_init_data,
			1,
			task_handle
		);
	}
}
