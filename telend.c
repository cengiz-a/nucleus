#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "methods.h"

void sig_handler(int signo)
{
  if (signo == SIGINT) {
    printf("received SIGINT\n");
  }

  exit(0);
}


int main() {
  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    printf("\ncan't catch SIGINT\n");
  }
  struct sockaddr_in server;
  memset( &server, 0, sizeof (server));

  unsigned long addr = inet_addr( "127.0.0.1");
  memcpy( (char *)&server.sin_addr, &addr, sizeof(addr));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);

  // Erzeuge das Socket - Verbindung über TCP/IP
  int sock = socket( AF_INET, SOCK_STREAM, 0 );
  if (sock < 0) {
    // Fehler beim Erzeugen des Sockets
  }
  // Baue die Verbindung zum Server auf.
  if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
    printf("No connection motherfucker\n");
  }
  char input[BUFFER_LENGTH];
  while(1) {
    fgets(input, BUFFER_LENGTH, stdin);
    send(sock, input, BUFFER_LENGTH, 0);
    recv(sock, input, BUFFER_LENGTH, 0);
    printf("%s\n", input);
  }
}
