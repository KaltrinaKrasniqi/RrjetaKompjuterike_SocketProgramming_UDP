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
