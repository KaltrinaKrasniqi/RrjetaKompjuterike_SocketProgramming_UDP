#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

#define BUFLEN 512
#define PORT 8888


typedef enum {
    READ_ONLY = 0,
    WRITE = 1,
    EXECUTE = 2,
    FULL_ACCESS = 3
} client_permission;

int main() {
    WSADATA wsa;
    printf("Initialising Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Initialised.\n");


    SOCKET client_socket;
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Socket created.\n");

   
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    }
 int permission;
    printf("Enter your permission level (0=ReadOnly, 1=Write, 2=Execute, 3=FullAccess): ");
    scanf("%d", &permission);
    getchar();

   
    if (permission < 0 || permission > 3) {
        printf("Invalid permission level. Exiting...\n");
        return 1;
    }

    client_permission userPermission = (client_permission)permission;

    while (1) {
       
        char command[BUFLEN];
        printf("Enter message: ");
        fgets(command, BUFLEN, stdin);

     
        command[strcspn(command, "\n")] = '\0';

       
        if (userPermission == FULL_ACCESS || userPermission == EXECUTE) {
            if (strcmp(command, "exit") == 0) {
                printf("Exiting...\n");
                break;
            }
            printf("Sending command to execute on server...\n");
            if (sendto(client_socket, command, strlen(command), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
                printf("sendto() failed with error code: %d\n", WSAGetLastError());
                return 1;
            }
        }

       
        if (userPermission == FULL_ACCESS || userPermission == WRITE) {
            printf("Sending message to server...\n");
            if (sendto(client_socket, command, strlen(command), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
                printf("sendto() failed with error code: %d\n", WSAGetLastError());
                return 1;
            }
        }
        else if (userPermission == READ_ONLY) {
            printf("You have read-only access. You can only receive messages.\n");
           
            if (sendto(client_socket, command, strlen(command), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
                printf("sendto() failed with error code: %d\n", WSAGetLastError());
                return 1;
            }
        }

       
        char serverResponse[BUFLEN];
        int serverResponseLength = recvfrom(client_socket, serverResponse, BUFLEN, 0, NULL, NULL);

       
        if (serverResponseLength == SOCKET_ERROR) {
            printf("recvfrom() failed with error code: %d\n", WSAGetLastError());
            return 1;
        }

        serverResponse[serverResponseLength] = '\0';
        printf("Server says: %s\n", serverResponse);
    }


    closesocket(client_socket);
    WSACleanup();
    return 0;
}
