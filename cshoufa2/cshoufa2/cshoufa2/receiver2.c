#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")  // Link with the Ws2_32.lib library

void show_message(const char* message);
DWORD WINAPI handle_client(LPVOID socket_desc);

int main(int argc, char* argv[]) {
    WSADATA wsa;
    SOCKET socket_desc, client_sock;
    int c;
    struct sockaddr_in server, client;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    show_message("Winsock initialized");

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
        return 1;
    }
    show_message("Socket created");

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8001);

    // Bind
    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return 1;
    }
    show_message("Bind done");

    // Listen
    listen(socket_desc, 3);

    // Accept incoming connections
    show_message("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr*)&client, &c)) != INVALID_SOCKET) {
        HANDLE thread;
        SOCKET* new_sock = malloc(sizeof(SOCKET));
        if (new_sock == NULL) {
            perror("Memory allocation failed");
            continue;  // Skip this iteration if memory allocation fails
        }
        *new_sock = client_sock;

        thread = CreateThread(NULL, 0, handle_client, (void*)new_sock, 0, NULL);
        if (thread == NULL) {
            perror("could not create thread");
            free(new_sock);  // Make sure to free memory if thread creation fails
            return 1;
        }

        show_message("Connection accepted");
    }

    if (client_sock == INVALID_SOCKET) {
        printf("accept failed with error code : %d", WSAGetLastError());
        return 1;
    }

    WSACleanup();
    return 0;
}

//DWORD WINAPI handle_client(LPVOID socket_desc) {
//    SOCKET sock = *(SOCKET*)socket_desc;
//    int read_size;
//    char client_message[2000];
//
//    // Receive a message from client
//    while ((read_size = recv(sock, client_message, 2000, 0)) > 0) {
//        // Send the message back to client
//        send(sock, client_message, strlen(client_message), 0);
//    }
//
//    if (read_size == 0) {
//        show_message("Client disconnected");
//        fflush(stdout);
//    }
//    else if (read_size == SOCKET_ERROR) {
//        perror("recv failed");
//    }
//
//    // Free the socket pointer
//    free(socket_desc);
//
//    return 0;
//}

DWORD WINAPI handle_client(LPVOID socket_desc) {
    SOCKET sock = *(SOCKET*)socket_desc;
    int read_size;
    char client_message[2000];

    // Receive a message from client
    while ((read_size = recv(sock, client_message, sizeof(client_message) - 1, 0)) > 0) {
        // Ensure the message is null-terminated
        client_message[read_size] = '\0';
        printf("Received message: %s\n", client_message);  // Display the message
    }

    if (read_size == 0) {
        printf("Client disconnected.\n");
    }
    else if (read_size == SOCKET_ERROR) {
        printf("recv failed with error: %d\n", WSAGetLastError());
    }

    free(socket_desc);
    return 0;
}


void show_message(const char* message) {
    printf("%s\n", message);
}
