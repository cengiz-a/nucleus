#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "methods.h"

#define PORT 4711

struct KeyValue res[1024];

int main() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);

  int clientSocket;

  if(sock < 0) {
    printf("Socket konnte nicht erstellt werden. \n");
  }
  // Serversocket
  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  
  // Clientsocket
  struct sockaddr_in client;

  if(bind(sock, (struct sockaddr *) &server, sizeof(server)) > 0) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if(listen(sock, 5) == -1) {
    perror("Cannot listen at socket");
    exit(EXIT_FAILURE);
  }

  while(1) {
    int len = sizeof(client);
    clientSocket = accept(sock, (struct sockaddr*)&client, &len);

    if(clientSocket < 0) {
      perror("Cannot accept socket");
      exit(EXIT_FAILURE);
    }

    printf("Client connected\n");
  }

  printf("hello\n");
}
