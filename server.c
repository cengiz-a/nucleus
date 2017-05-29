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

#define PORT 4711
#define BUFFER_LENGTH 256
#define STORE_SIZE 1024
#define KEY_LENGTH 64
#define MAX_CMD 3

int parseMessage(char* message, char parsed[MAX_CMD][KEY_LENGTH]) {
  int c_index = 0;
  int k_index = 0;

  while(*message) {
    // Check for space
    if(*message == 32) {
      // Move to next cmd
      c_index++;
      // Move to next character
      message++;
      // Start from start again
      k_index = 0;
    } else {
      // Copy character and move to next
      parsed[c_index][k_index++] = *(message++);
    }
  }
  // TODO: Validate message and return errorcode
  // TODO: allow spaces inside value -> "
  return 0;
}

int main() {
  int id = shmget(IPC_PRIVATE, sizeof(struct KeyValue) * STORE_SIZE, IPC_CREAT|0600);
  struct shmid_ds status;

  shmctl(id, IPC_STAT, &status);
  printf("Last attach: %d\n", status.shm_atime);

  // Attach
  struct KeyValue* pointer_to_shared_memory = shmat(id, 0, 0);
  memset(pointer_to_shared_memory, 0, sizeof(pointer_to_shared_memory));
  strcpy(pointer_to_shared_memory[0].key, "1337");
  printf("Pointer: %p\n", pointer_to_shared_memory);

  shmctl(id, IPC_STAT, &status);
  printf("Last attach: %d\n", status.shm_atime);

  int pid = fork();
  printf("shared memory: %s\n", pointer_to_shared_memory[0].key);

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
  int clientSocket = accept(serverSocket, (struct sockaddr*)&client, &len);

  if(clientSocket < 0) {
    perror("Cannot accept socket");
    exit(EXIT_FAILURE);
  }

  printf("Client connected\n");

  char buffer[BUFFER_LENGTH];
  memset(buffer, 0, sizeof(buffer));

  while(recv(clientSocket, buffer, BUFFER_LENGTH, 0)) {
    int ret = parseMessage(buffer, parsed);

    if(ret < 0) {
      printf("Error while parsing message\n");
    }

    printf("CMD: %s, KEY: %s, VAL: %s\n", parsed[0], parsed[1], parsed[2]);

    if(!strcmp(parsed[0], "GET")) {
      // get value by key and return it
      printf("GET\n");
    } else if(!strcmp(parsed[0], "PUT")) {
      // set value for key and return old value if it was set
      put(parsed[1], parsed[2], pointer_to_shared_memory);
      printf("PUT %s %s", pointer_to_shared_memory[0].key, pointer_to_shared_memory[0].value);
    } else if(!strcmp(parsed[0], "DEL")) {
      // delete key
      printf("DEL\n");
    } else {
      shutdown(clientSocket, 0);
    }
  }
}
