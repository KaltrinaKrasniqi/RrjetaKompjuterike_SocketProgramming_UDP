#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

#define BUFLEN 512
#define MAX_CLIENTS 5
#define PORT 8888
#define TIMEOUT 30
#define WAITING_TIMEOUT 10
#define READ_DELAY 3000 // Delay per read-only klient

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
    int read_only;  // Flag per me shiku a eshte klienti read-only
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


void delayA(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
    clock_t start_time = clock();
    while (clock() < start_time + milli_seconds)
        ;
}

void handle_client_message(SOCKET client_socket, char* command, struct sockaddr_in* client_addr, Client* client) {
    char response[BUFLEN];

    if (client->read_only && strncmp(command, "read", 4) == 0) {
        printf("Delaying read request for read-only client...\n");
        delayA(5);  
    }

    if (strncmp(command, "read", 4) == 0) {

        char file_name[BUFLEN];
        sscanf(command, "read %[^\n]", file_name);
        char path[BUFLEN] = "C:\\Users\\kaltr\\source\\repos\\RrjetaKompjuterike_SocketProgramming_UDP\\Server\\";
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
        
        int num_fields = sscanf(command, "write %s %[^\n]", filename, content);

        if (num_fields < 2) {
            snprintf(response, sizeof(response), "Error: Invalid write command.");
            sendto(client_socket, response, strlen(response), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
            return;
        }
        char path[BUFLEN] = "C:\\Users\\kaltr\\source\\repos\\RrjetaKompjuterike_SocketProgramming_UDP\\Server\\";
        strcat(path, filename);

        
        FILE* file = fopen(path, "a+");
        if (file) {
            
            fprintf(file, "%s\n", content);
            fclose(file);

            snprintf(response, sizeof(response), "Content successfully written to %s", filename);
            sendto(client_socket, response, strlen(response), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
        }
        else {
            
            perror("Error opening file");
            snprintf(response, sizeof(response), "Failed to open file for writing. File may not exist.");
            sendto(client_socket, response, strlen(response), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
        }
    }

    else if (strncmp(command, "mkdir", 5) == 0) {

        
        char dir_name[BUFLEN];
        strcpy(dir_name, command + 6); 

        
        int start = 0;
        while (dir_name[start] == ' ') start++; 
        int end = strlen(dir_name) - 1;
        while (end >= 0 && dir_name[end] == ' ') end--; 
        printf("Directory name after trimmingggggggggg: '%s'\n", dir_name);
       
        if (start > end) {
            snprintf(response, sizeof(response), "Error: No directory name provided.");
            sendto(client_socket, response, strlen(response), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
            return;
        }
        printf("Directory name after trimming: '%s'\n", dir_name);
        
        dir_name[end + 1] = '\0';  

       
        char path[BUFLEN] = "C:\\Users\\kaltr\\source\\repos\\RrjetaKompjuterike_SocketProgramming_UDP\\Server\\";
        if (strlen(path) + strlen(dir_name) + 1 < BUFLEN) {
            strcat(path, dir_name);  
        }
        else {
            snprintf(response, sizeof(response), "Error: Path is too long.");
            sendto(client_socket, response, strlen(response), 0, (struct sockaddr*)client_addr, sizeof(*client_addr));
            return;
        }
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
    printf("Initializing Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d", WSAGetLastError());
        exit(0);
    }
    printf("Initialized.\n");

    SOCKET server_socket;
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR) {
        printf("Error in socket creation: %d", WSAGetLastError());
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed with error code %d", WSAGetLastError());
        return -1;
    }

    printf("Server listening on port %d...\n", PORT);

    Client clients[MAX_CLIENTS];
    Client waiting_clients[MAX_CLIENTS];
    int clientCount = 0;
    int waiting_client_count = 0;

    while (1) {
        char buffer[BUFLEN];
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        // Marrja e data from client
        int recv_len = recvfrom(server_socket, buffer, BUFLEN, 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (recv_len == SOCKET_ERROR) {
            printf("Recvfrom failed with error code %d", WSAGetLastError());
            continue;
        }

        buffer[recv_len] = '\0';  // Null-terminate received data
        printf("Received message: %s\n", buffer);

        // Log request
        logRequest(inet_ntoa(client_addr.sin_addr), buffer);

        // Handle client request
        if (isClientRegistered(clients, clientCount, &client_addr)) {
            handle_client_message(server_socket, buffer, &client_addr);
        }
        else {
            if (clientCount < MAX_CLIENTS) {
                clients[clientCount].addr = client_addr;
                clients[clientCount].socket = server_socket;
                clients[clientCount].last_active = time(NULL);
                clientCount++;
                handle_client_message(server_socket, buffer, &client_addr);
            }
            else {
                // Add to waiting clients if the server is full
                waiting_clients[waiting_client_count].addr = client_addr;
                waiting_clients[waiting_client_count].socket = server_socket;
                waiting_clients[waiting_client_count].last_active = time(NULL);
                waiting_client_count++;
                sendto(server_socket, "Server is full. You are added to the waiting list.", 51, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
            }
        }

        cleanInactiveClients(clients, &clientCount, waiting_clients, &waiting_client_count);
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}
