#ifndef WINDOWS_SOCKET_H
#define WINDOWS_SOCKET_H

#include "../../../Library/frame.h"


SOCKET open_tcp_connection();

void send_data_to_server(
    SOCKET endpoint,
	frame_size_t frame_size,
	long* buffer_to_transmit
);

//void close_tcp_connection(SOCKET endpoint);

#endif // !WINDOWS_SOCKET_H
