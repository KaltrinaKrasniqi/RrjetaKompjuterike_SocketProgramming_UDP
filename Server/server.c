#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

#define BUFLEN 512
#define MAX_CLIENTS 3
#define PORT 8888
#define TIMEOUT 30
#define WAITING_TIMEOUT 10

// funksioni me append content ne file
void appendFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "a");
    if (file != NULL) {
        fputs(content, file);
        fclose(file);
    }
    else {
        printf("Error opening log file!\n");
    }
}

// funksioni me logu requestat dhe ip-t e klientit
void logRequest(const char* client_ip, const char* request) {
    time_t current_time;
    char* time_str;
    time(&current_time);
    time_str = ctime(&current_time);
    time_str[strlen(time_str) - 1] = '\0';
    char log_entry[BUFLEN];
    snprintf(log_entry, sizeof(log_entry), "[%s] Request from %s: %s\n", time_str, client_ip, request);
    appendFile("server_log.txt", log_entry);
}

// Struktura per menaxhimin e lidhjeve te klientit
typedef struct {
    struct sockaddr_in addr;
    time_t last_active;
    SOCKET socket;
} Client;


// shikimi se a eshte klienti i regjistruar
int isClientRegistered(Client clients[], int clientCount, struct sockaddr_in* clientAddr) {
    for (int i = 0; i < clientCount; i++) {
        if (clients[i].addr.sin_addr.s_addr == clientAddr->sin_addr.s_addr &&
            clients[i].addr.sin_port == clientAddr->sin_port) {
            clients[i].last_active = time(NULL); // Update last active time
            return 1;
        }
    }
    return 0;
}

// Funksion per largimin e klienteve joaktiv
void cleanInactiveClients(Client clients[], int* clientCount, Client waiting_clients[], int* waiting_client_count) {
    time_t current_time = time(NULL);
    for (int i = 0; i < *clientCount; i++) {
        if (difftime(current_time, clients[i].last_active) > TIMEOUT) {

            printf("Client %s is inactive. Closing connection.\n", inet_ntoa(clients[i].addr.sin_addr));
            closesocket(clients[i].socket);


            for (int j = i; j < *clientCount - 1; j++) {
                clients[j] = clients[j + 1];
            }
            (*clientCount)--;
            i--;
        }
    }

    // Kontrollo dhe merr nje klient nga queue nese lirohet vendi
    if (*clientCount < MAX_CLIENTS && *waiting_client_count > 0) {

        clients[*clientCount] = waiting_clients[0];
        (*clientCount)++;
        for (int i = 0; i < *waiting_client_count - 1; i++) {
            waiting_clients[i] = waiting_clients[i + 1];
        }
        (*waiting_client_count)--;

        char message[BUFLEN];
        snprintf(message, sizeof(message), "You are now active. Welcome!");
        sendto(clients[*clientCount - 1].socket, message, strlen(message), 0, (struct sockaddr*)&clients[*clientCount - 1].addr, sizeof(clients[*clientCount - 1].addr));
    }
}

void handle_client_message(SOCKET client_socket, char* command, struct sockaddr_in* client_addr) {
    char response[BUFLEN];


    if (strncmp(command, "read", 4) == 0) {
        char file_name[BUFLEN];
        sscanf(command, "read %[^\n]", file_name);
        char path[BUFLEN] = "C:\\Users\\kaltr\\source\\repos\\Prova\\Prova\\";
        strcat(path, file_name);

        FILE* file = fopen(path, "r");
        if (file) {
            char file_content[BUFLEN];
            size_t bytes_read = fread(file_content, 1, sizeof(file_content) - 1, file);
            file_content[bytes_read] = '\0';

            snprintf(response, sizeof(response), "File content:\n%s", file_content);
            fclose(file);
        }
        else {
            snprintf(response, sizeof(response), "Failed to open file %s.", file_name);
        }
        sendto(client_socket, response, strlen(response), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
    }

    else if (strncmp(command, "write", 5) == 0) {
        char filename[BUFLEN], content[BUFLEN];
        sscanf(command, "write %[^\n] %[^\n]", filename, content);

        char path[BUFLEN] = "C:\\Users\\kaltr\\source\\repos\\Prova\\Prova\\";
        strcat(path, filename);

        FILE* file = fopen(path, "r+");
        if (file) {

            fseek(file, 0, SEEK_END);
            fprintf(file, "%s\n", content);
            fclose(file);


            sendto(client_socket, content, strlen(content), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
        }
        else {

            perror("Error opening file");
            snprintf(response, sizeof(response), "Failed to open file for writing. File may not exist.");
            sendto(client_socket, response, strlen(response), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
        }
    }



    else if (strncmp(command, "mkdir", 5) == 0) {

        char dir_name[BUFLEN];
        sscanf(command + 6, "%[^\n]", dir_name);

        char path[BUFLEN] = "C:\\Users\\kaltr\\source\\repos\\Prova\\Prova\\";
        strcat(path, dir_name);

        if (_mkdir(path) == 0) {
            snprintf(response, sizeof(response), "Directory '%s' created successfully.", dir_name);
        }
        else {
            snprintf(response, sizeof(response), "Failed to create directory '%s'.", dir_name);
        }

        response[sizeof(response) - 1] = '\0';

        sendto(client_socket, response, strlen(response), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
    }

    else {
        snprintf(response, sizeof(response), "Unknown command.");
        sendto(client_socket, response, strlen(response), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
    }

}

int main() {
    system("title UDP Server");

    WSADATA wsa;
    printf("Initialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d", WSAGetLastError());
        exit(0);
    }
    printf("Initialised.\n");

    SOCKET server_socket;
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d", WSAGetLastError());
    }
    printf("Socket created.\n");

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code: %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    puts("Bind done.");

    // Lista e klientëve të regjistruar
    Client clients[MAX_CLIENTS];
    int clientCount = 0;

    // Lista për klientët në pritje
    Client waiting_clients[MAX_CLIENTS];
    int waiting_client_count = 0;

    // Timeout setup
    fd_set readfds;
    struct timeval timeout;

    while (1) {
        printf("Waiting for data...\n");
        fflush(stdout);

        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);

        int select_result = select(0, &readfds, NULL, NULL, &timeout);
        if (select_result == 0) {
            printf("No data received for %d seconds. Checking connections...\n", TIMEOUT);
            cleanInactiveClients(clients, &clientCount, waiting_clients, &waiting_client_count);
            continue;
        }

        char message[BUFLEN] = { 0 };
        struct sockaddr_in clientAddr;
        int slen = sizeof(clientAddr);
        int message_len;

        if ((message_len = recvfrom(server_socket, message, BUFLEN, 0, (struct sockaddr*)&clientAddr, &slen)) == SOCKET_ERROR) {
            printf("recvfrom() failed with error code: %d", WSAGetLastError());
            continue;
        }

        // Log the request with timestamp and IP
        logRequest(inet_ntoa(clientAddr.sin_addr), message);

        // Kontrollo nëse serveri është plot
        if (clientCount < MAX_CLIENTS) {
            // Lejo klientin të lidhet
            printf("New client connected from %s\n", inet_ntoa(clientAddr.sin_addr));

            // Krijo socket për klientin
            SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);
            if (client_socket == INVALID_SOCKET) {
                printf("Error creating socket for client.\n");
                continue;
            }

            // Regjistro klientin dhe lidhje socketin me të
            clients[clientCount].addr = clientAddr;
            clients[clientCount].last_active = time(NULL);
            clients[clientCount].socket = client_socket; // Save the client's socket
            clientCount++;

            // Dërgo një mesazh klientit për lidhjen
            char response[BUFLEN];
            snprintf(response, sizeof(response), "You are now connected. Welcome!");
            sendto(server_socket, response, strlen(response), 0, (struct sockaddr*)&clientAddr, sizeof(struct sockaddr_in));
        }
        else {
            // Serveri është plot, vendos klientin në waiting queue
            printf("Server is full. Adding client to waiting queue from %s\n", inet_ntoa(clientAddr.sin_addr));

            waiting_clients[waiting_client_count].addr = clientAddr;
            waiting_clients[waiting_client_count].last_active = time(NULL);
            waiting_client_count++;

            // Dërgo mesazh që klienti është në pritje
            char waitingMsg[BUFLEN];
            snprintf(waitingMsg, sizeof(waitingMsg), "Server is full. You are in the waiting queue.");
            sendto(server_socket, waitingMsg, strlen(waitingMsg), 0, (struct sockaddr*)&clientAddr, sizeof(struct sockaddr_in));
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

