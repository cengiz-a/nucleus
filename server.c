#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "methods.h"


// parseMessage fängt die Eingabe ab und teilt es nach Leerzeichen auf.
// Die einzelnen Wörter werden in einem Array gespeichert.
int parseMessage(char* message, char parsed[MAX_CMD][KEY_LENGTH]) {
  int word_index = 0;
  int char_index = 0;

  while(*message) {
    // Check for space
    if(*message == 32) {
      // Move to next cmd
      word_index++;
      // Move to next character
      message++;
      // Start from start again
      char_index = 0;
    } else if(*message == 10 || *message == 13) {
      // Ignore carriage return and newline
      char_index++;
      message++;
    } else {
      // Copy character and move to next
      parsed[word_index][char_index++] = *(message++);
    }
  }

  // TODO: Validate message and return errorcode
  // TODO: allow spaces inside value -> "
  return 0;
}

int main() {
  // anlegen eines shared_memory
  int id = shmget(IPC_PRIVATE, sizeof(struct KeyValue) * STORE_SIZE, IPC_CREAT|0600);

  // Attac
  struct KeyValue* pointer_to_shared_memory = shmat(id, 0, 0);
  memset(pointer_to_shared_memory, 0, sizeof(pointer_to_shared_memory));

  char parsed[MAX_CMD][KEY_LENGTH];
  memset(parsed, 0, sizeof(parsed));

  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

  if(serverSocket < 0) {
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

  if(bind(serverSocket, (struct sockaddr *) &server, sizeof(server)) > 0) {
    perror("Could not bind socket");
    exit(EXIT_FAILURE);
  }

  if(listen(serverSocket, 5) == -1) {
    perror("Cannot listen at socket");
    exit(EXIT_FAILURE);
  }

  int len = sizeof(client);
  int clientSocket;

  while(clientSocket = accept(serverSocket, (struct sockaddr*)&client, &len)) {
      printf("Client connected, %d\n", clientSocket);
      int pid = fork();
      if(pid == 0) {
        if(clientSocket < 0) {
          perror("Cannot accept socket");
          exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_LENGTH];
        memset(buffer, 0, sizeof(buffer));

        while(recvLen = recv(clientSocket, buffer, BUFFER_LENGTH, 0)) {
          int ret = parseMessage(buffer, parsed);

          if(ret < 0) {
            printf("Error while parsing message\n");
          }

          if(!strcmp(parsed[0], "GET")) {
            // get value by key and return it
            char returnValue[KEY_LENGTH];
            memset(returnValue, 0, KEY_LENGTH);
            get(parsed[1], returnValue, pointer_to_shared_memory);
            send(clientSocket, returnValue, KEY_LENGTH, 0);
            printf("GET %s %s\n", parsed[1], returnValue);
          } else if(!strcmp(parsed[0], "PUT")) {
            // set value for key and return old value if it was set
            put(parsed[1], parsed[2], pointer_to_shared_memory);
            printf("PUT %s %s\n", pointer_to_shared_memory[0].key, pointer_to_shared_memory[0].value);
          } else if(!strcmp(parsed[0], "DEL")) {
            // delete key
            del(parsed[1], pointer_to_shared_memory);
            printf("DEL %s\n", parsed[1]);
          } else {
            shutdown(clientSocket, 0);
          }

          memset(buffer, 0, sizeof(buffer));
          memset(parsed, 0, sizeof(parsed));
        }
      }
  }
}
