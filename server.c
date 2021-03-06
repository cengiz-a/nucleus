#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/sem.h>
#include<signal.h>

#include "methods.h"

struct KeyValue* pointer_to_shared_memory;
int sem_id;
int sem_id_read;

void sig_handler(int signo)
{
  if (signo == SIGINT) {
    printf("received SIGINT\n");
  }

  FILE *f = fopen("keyvalue_storage", "w");
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(1);
  }

  fwrite(pointer_to_shared_memory, sizeof(struct KeyValue), STORE_SIZE, f);
  fclose(f);
  // cleanup
  shmdt(pointer_to_shared_memory);
  semctl(sem_id, 1, IPC_RMID);
  semctl(sem_id_read, 1, IPC_RMID);
  exit(0);
}

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
  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    printf("\ncan't catch SIGINT\n");
  }
  // anlegen eines shared_memory
  int *reader = NULL;
  int id = shmget(IPC_PRIVATE, sizeof(struct KeyValue) * STORE_SIZE + sizeof(int), IPC_CREAT|0600);
  sem_id = semget (IPC_PRIVATE, 1, IPC_CREAT);
  //int reader = 0;
  sem_id_read = semget(IPC_PRIVATE, 1, IPC_CREAT);
  struct sembuf semaphore_lock[1]   = { 0, -1, SEM_UNDO };
  struct sembuf semaphore_unlock[1] = { 0, 1,  SEM_UNDO };

  int valone = 1;

  if (sem_id < 0) {
      umask(0);
      sem_id = semget (IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0666);
      if (sem_id < 0) {
         printf ("Fehler beim Anlegen des Semaphors ... FEHLER: %d\n", sem_id);
         return -1;
      }
      printf ("(angelegt) Semaphor-ID : %d\n", sem_id);
      /* Semaphor mit 1 initialisieren */
      if (semctl (sem_id, 0, SETALL, &valone) == -1)
         return -1;
   }


  if(sem_id_read < 0) {
    sem_id_read = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0666);
    if(sem_id_read < 0) {
    	printf("Fehler beim Anlegen des read Semaphores...\n");
	    return -1;
    }
    printf("(angelegt) Semaphor-Read-ID: %d\n", sem_id_read);
    if(semctl(sem_id_read, 0, SETALL, &valone) == -1)
	    return -1;
  }

  // Attach
  pointer_to_shared_memory = shmat(id, 0, 0);
  memset(pointer_to_shared_memory, 0, sizeof(pointer_to_shared_memory));

  FILE *f = fopen("keyvalue_storage", "r+b");
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(1);
  }
  int re = fread(pointer_to_shared_memory, sizeof(struct KeyValue), STORE_SIZE, f);
  fclose(f);

  reader = (int*) (pointer_to_shared_memory + STORE_SIZE);
  printf("Address of KeyValue-Store: %p \n", pointer_to_shared_memory);
  printf("Sizeof KeyValue: %d \n", (int)sizeof(struct KeyValue));
  printf("Store-Size: %d \n", STORE_SIZE);
  printf("Address of reader: %p \n", reader);

  if(pointer_to_shared_memory < 0) {
      printf("Could not attach shared memory.\n");
      return -1;
  }

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

         if(sem_id > 0) {
           printf("Semaphore mit der ID: %d wird benutzt\n", sem_id);
         }

        char buffer[BUFFER_LENGTH];
        memset(buffer, 0, sizeof(buffer));

        int recvLen = 0;

        while(recvLen = recv(clientSocket, buffer, BUFFER_LENGTH, 0)) {
          printf("RECIEVED: %s\n", buffer);
          int ret = parseMessage(buffer, parsed);

          if(ret < 0) {
            printf("Error while parsing message\n");
          }

          if(!strcmp(parsed[0], "GET")) {
              // get value by key and return it
              char retVal[KEY_LENGTH];
              memset(retVal, 0, KEY_LENGTH);
  	          semop(sem_id_read, &semaphore_lock[0], 1);

  	          (*reader)++;

  	          if(*reader == 1) {
                semop(sem_id, &semaphore_lock[0], 1);
	  	        }

        	    semop(sem_id_read, &semaphore_unlock[0], 1);

              int ret = get(parsed[1], retVal, pointer_to_shared_memory);

        	    semop(sem_id_read, &semaphore_lock[0], 1);

        	    // Decrement read counter
        	    (*reader)--;

        	    // Last read thread unlocks write semaphore
        	    if(*reader == 0) {
                      semop(sem_id, &semaphore_unlock[0], 1 );
        	    }



  	          if(ret < 0) {
                send(clientSocket, "Wert nicht gefunden!", 20, 0);
              } else {
                send(clientSocket, retVal, KEY_LENGTH, 0);
              }

              printf("GET %s %s\n", parsed[1], retVal);
              fflush(stdout);
              sleep(5);
              semop(sem_id_read, &semaphore_unlock[0], 1);
          } else if(!strcmp(parsed[0], "PUT")) {
                // set value for key and return old value if it was set
                char retVal[KEY_LENGTH];
                memset(retVal, 0, KEY_LENGTH);
                /* Ressource sperren */
                semop(sem_id, &semaphore_lock[0], 1);
    	          int ret = put(parsed[1], parsed[2], retVal, pointer_to_shared_memory);
                /* Ressource freigeben */
                semop(sem_id, &semaphore_unlock[0], 1);
                send(clientSocket, retVal, KEY_LENGTH, 0);
            } else if(!strcmp(parsed[0], "DEL")) {
                // delete key
                char retVal[KEY_LENGTH];
                memset(retVal, 0, KEY_LENGTH);
                /* Ressource sperren */
                semop(sem_id, &semaphore_lock[0], 1);
                int ret = del(parsed[1], retVal, pointer_to_shared_memory);
                /* Ressource freigeben */
                semop(sem_id, &semaphore_unlock[0], 1 );
                if(ret < 0) {
                  send(clientSocket, "Wert nicht gefunden!", 20, 0);
                } else {
                  send(clientSocket, retVal, KEY_LENGTH, 0);
                }
                printf("DEL %s\n", parsed[1]);
            } else if(!strcmp(parsed[0], "exit")) {
              shutdown(clientSocket, 0);
          }

          memset(buffer, 0, sizeof(buffer));
          memset(parsed, 0, sizeof(parsed));
        }
      }
  }
}
