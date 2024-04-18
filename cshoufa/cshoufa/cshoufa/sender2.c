#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8001

DWORD WINAPI receive_message(void* socket_desc);
void send_messages(SOCKET sock);
void show_message(const char* message);

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    HANDLE recv_thread;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket created\n");

    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server.sin_addr) <= 0) {
        printf("Invalid address/Address not supported\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Connect to remote server
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Connect failed. Error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected\n");

    // Receive a message from server in a new thread
    recv_thread = CreateThread(NULL, 0, receive_message, &sock, 0, NULL);
    if (recv_thread == NULL) {
        printf("Could not create thread. Error: %d\n", GetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Keep sending messages to server
    send_messages(sock);

    WaitForSingleObject(recv_thread, INFINITE);
    CloseHandle(recv_thread);
    closesocket(sock);
    WSACleanup();
    return 0;
}

DWORD WINAPI receive_message(void* socket_desc) {
    SOCKET sock = *(SOCKET*)socket_desc;
    int read_size;
    char server_reply[2000];

    // Receive a reply from the server
    while ((read_size = recv(sock, server_reply, sizeof(server_reply) - 1, 0)) > 0) {
        server_reply[read_size] = '\0';  // Ensure null-termination
        show_message(server_reply);
    }

    if (read_size == 0) {
        puts("Server disconnected");
    }
    else if (read_size == SOCKET_ERROR) {
        printf("recv failed with error: %d\n", WSAGetLastError());
    }

    return 0;
}

//void send_messages(SOCKET sock) {
//    char message[2000];
//    int j;
//
//    for (j = 0; j < 50; j++) {
//        int len = sprintf_s(message, sizeof(message), "Message %d", j);
//        if (len > 0) {
//            message[len] = '\0';  // Explicitly null-terminate the string
//            if (send(sock, message, len, 0) < 0) {
//                printf("Send failed with error: %d\n", WSAGetLastError());
//                return;
//            }
//        }
//    }
//}
void send_messages(SOCKET sock) {
    char message[2000];
    while (1) {
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);

        // Remove newline character if present
        size_t len = strlen(message);
        if (len > 0 && message[len - 1] == '\n') {
            message[len - 1] = '\0';
            len--;
        }

        if (len == 0) continue;  // Skip empty messages

        if (send(sock, message, len, 0) < 0) {
            printf("Send failed with error: %d\n", WSAGetLastError());
            return;
        }
    }
}

void show_message(const char* message) {
    printf("%s\n", message);
}
