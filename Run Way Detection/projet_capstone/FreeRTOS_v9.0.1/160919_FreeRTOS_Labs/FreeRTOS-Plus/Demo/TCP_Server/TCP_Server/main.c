#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../../Library/console.h"
#include "../../Library/frame.h"
#include "../../Library/rsa.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_PORT "7878"


int __cdecl main(void)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    //int iSendResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        log("WSAStartup failed with error: %d", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        log("getaddrinfo failed with error: %d", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        log("socket failed with error: %ld", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        log("bind failed with error: %d", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        log("listen failed with error: %d", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    log("the TCP server is waiting for connections");
    log("you may now open the FreeRTOS client");

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        log("accept failed with error: %d", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    ///closesocket(ListenSocket);

    size_t data_size;
    unsigned short saved_decrypted_sample = 0;

    while (1) {
        size_t remaining_bytes = sizeof(size_t);
        int read_bytes = 0;

        while (remaining_bytes > 0) {
            read_bytes = recv(
                ClientSocket,
                &((unsigned char*)&data_size)[sizeof(size_t) - remaining_bytes],
                remaining_bytes,
                0
            );
            remaining_bytes -= read_bytes;
        }

        long
            *encrypted_frame_wrapper = malloc(data_size),
            *decrypted_frame_wrapper = malloc(data_size);

        remaining_bytes = data_size;
        read_bytes = 0;

        while (remaining_bytes > 0) {
            read_bytes = recv(
                ClientSocket,
                &(((unsigned char*)encrypted_frame_wrapper)[data_size - remaining_bytes]),
                remaining_bytes,
                0
            );
            remaining_bytes -= read_bytes;
        }

        log("Received %u bytes", data_size);

        if (!saved_decrypted_sample) {
            frame_t frame;
            key_pair_t key_pair = generate_key_pair(167, 223);

            log("generated key pair");

            log("decrypting sample image...");
            decrypt(
                encrypted_frame_wrapper,
                decrypted_frame_wrapper,
                key_pair.private_key,
                key_pair.rsa_modulus,
                data_size / sizeof(long),
                0, 0
            );

            log("decrypted sample image");

            size_t i, header_size = sizeof(frame.ih);

            for (i = 0; i < header_size; ++i)
                ((unsigned char*)&frame.ih)[i] = decrypted_frame_wrapper[i];

            size_t bitmap_size = frame.ih.bmp_bytesz * sizeof(pixel_t);
            frame.bitmap_data = malloc(bitmap_size);

            for (; (unsigned)(i - header_size) < bitmap_size; ++i)
                ((unsigned char*)frame.bitmap_data)[i - header_size] = decrypted_frame_wrapper[i];

            if (save_bmp(
                "decrypted-lena.bmp",
                &frame.ih,
                frame.bitmap_data
            )) {
                log("decrypted frame sample not saved");
            }
            else {
                saved_decrypted_sample = 1;
                log("stored decrypted frame sample as: decrypted-lena-runway.bmp");
            }

            free(frame.bitmap_data);
        }

        free(encrypted_frame_wrapper);
    }

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        log("shutdown failed with error: %d", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}
