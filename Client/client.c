#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

#define BUFLEN 512
#define PORT 8888


typedef enum {
    READ_ONLY = 0,
    FULL_ACCESS = 3
} client_permission;

void read_file(SOCKET client_socket, struct sockaddr_in server_addr) {
    char filename[BUFLEN];
    printf("Enter the filename to read (relative path from server directory): ");
    fgets(filename, BUFLEN, stdin);
    filename[strcspn(filename, "\n")] = '\0';

    // Send the read command to the server
    char command[BUFLEN];
    snprintf(command, sizeof(command), "read %s", filename);
    sendto(client_socket, command, strlen(command), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // Receive file content from server
    char response[BUFLEN];
    int response_length = recvfrom(client_socket, response, BUFLEN, 0, NULL, NULL);
    if (response_length > 0) {
        printf("File content:\n%s\n", response);
    }
    else {
        printf("Error receiving file content.\n");
    }
}

void write_file(SOCKET client_socket, struct sockaddr_in server_addr) {
    char filename[BUFLEN], content[BUFLEN];
    printf("Enter the filename to write to (relative path from server directory): ");
    fgets(filename, BUFLEN, stdin);
    filename[strcspn(filename, "\n")] = '\0';

    printf("Enter the content to write: ");
    fgets(content, BUFLEN, stdin);
    content[strcspn(content, "\n")] = '\0';

    // Send the write command to the server
    char command[BUFLEN];
    snprintf(command, sizeof(command), "write %s %s", filename, content);
    sendto(client_socket, command, strlen(command), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // Wait for server response
    char response[BUFLEN];
    recvfrom(client_socket, response, BUFLEN, 0, NULL, NULL);
    printf("%s\n", response);
}
////funskioni per me i handle klientat full_access
void handle_full_access_commands(SOCKET client_socket, struct sockaddr_in server_addr) {
    int choice;


    printf("\nChoose an operation:\n");
    printf("1. Read File\n");
    printf("2. Write File\n");
    printf("3. Execute Command (mkdir)\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    getchar();


    char command[BUFLEN];
    switch (choice) {
    case 1:
        read_file(client_socket, server_addr);
        break;
    case 2:
        write_file(client_socket, server_addr);
        break;
    case 3:
        printf("Enter directory name to create: ");
        fgets(command, BUFLEN, stdin);
        command[strcspn(command, "\n")] = '\0';// Remove newline

        if (strncmp(command, "mkdir ", 6) == 0) {
           
            char dir_name[BUFLEN];
            strcpy(dir_name, command + 6);  

           
            snprintf(command, sizeof(command), "mkdir %s", dir_name);  

           
            sendto(client_socket, command, strlen(command), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        }
        else {
           
            printf("Invalid command. Please start the command with 'mkdir'.\n");
        }
        break;
    case 4:
        printf("Exiting...\n");
        exit(0);
        break;
    default:
        printf("Invalid choice. Please select a valid operation.\n");
        return;
    }
}
//funskioni per me i handle funksionet  e klientave me read_access
void handle_read_only_access(SOCKET client_socket, struct sockaddr_in server_addr) {
    // Simulate system information for read-only users
    const char* system_info[] = {
        "System Architecture: x64",
        "OS Version: Windows 10 Pro",
        "CPU: Intel Core i7-9700K @ 3.60 GHz",
        "Memory: 16 GB DDR4",
        "Network IP: 192.168.1.10",
        "Disk Space: 500 GB SSD (350 GB free)",
        "Random Fact: The sky is blue due to Rayleigh scattering.",
        "Random Fact: The Eiffel Tower can grow by up to 15 cm in the summer due to expansion."
    };

    int num_info = sizeof(system_info) / sizeof(system_info[0]);
    srand(time(NULL)); //gjenerim i numrave random per mesazhe

    int random_index = rand() % num_info;

    // Display a random piece of system info
    printf("Random PC Information: %s\n", system_info[random_index]);

    // Give the user the option to read a file
    char command[BUFLEN];
    printf("\nEnter command (exit for Exit, read [filename] for file content): ");
    fgets(command, BUFLEN, stdin);


    command[strcspn(command, "\n")] = '\0';

    // Check for exit command
    if (strcmp(command, "exit") == 0) {
        printf("Exiting...\n");
        exit(0);
    }

    // Check for read command
    if (strncmp(command, "read", 4) == 0) {
        // Extract filename from command
        char filename[BUFLEN];
        sscanf(command, "read %[^\n]", filename);

        // Send read request to server
        int send_result = sendto(client_socket, command, strlen(command), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (send_result == SOCKET_ERROR) {
            printf("sendto() failed with error code: %d\n", WSAGetLastError());
            return;
        }

        // Receive file content from server
        char serverResponse[BUFLEN];
        int serverResponseLength = recvfrom(client_socket, serverResponse, BUFLEN, 0, NULL, NULL);

        if (serverResponseLength == SOCKET_ERROR) {
            printf("recvfrom() failed with error code: %d\n", WSAGetLastError());
        }
        else {
            serverResponse[serverResponseLength] = '\0';  // Null-terminate response
            printf("Server response: %s\n", serverResponse);
        }
    }
    else {
        printf("Unknown command. Try again.\n");
    }
}


int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock\n");
        return -1;
    }

    // Create socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Get username and password from client
    char username[50];
    char password[50];

    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    printf("Enter password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';


    client_permission userPermission;
    if (strcmp(username, "admin") == 0 && strcmp(password, "password123") == 0) {
        userPermission = FULL_ACCESS;
        printf("You are now connected. Welcome!\n");
        printf("You have full access. You can read, write, and execute commands.\n");
    }
    else {
        userPermission = READ_ONLY;
        printf("You are now connected. Welcome!\n");
        printf("You have Read-Only access. You cannot execute any commands.\n");
    }


    while (1) {
        if (userPermission == FULL_ACCESS) {
            handle_full_access_commands(client_socket, server_addr);
        }
        else if (userPermission == READ_ONLY) {
                  printf("Heeej");
            handle_read_only_access(client_socket, server_addr);
        }
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
