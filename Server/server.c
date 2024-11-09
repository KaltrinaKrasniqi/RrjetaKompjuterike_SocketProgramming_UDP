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
#define TIMEOUT 10 
#define WAITING_TIMEOUT 10 


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


typedef struct {
    struct sockaddr_in addr;
    time_t last_active;
    SOCKET socket;  
} Client;


int isClientRegistered(Client clients[], int clientCount, struct sockaddr_in* clientAddr) {
    for (int i = 0; i < clientCount; i++) {
        if (clients[i].addr.sin_addr.s_addr == clientAddr->sin_addr.s_addr &&
            clients[i].addr.sin_port == clientAddr->sin_port) {
            clients[i].last_active = time(NULL); 
            return 1;
        }
    }
    return 0;
}
