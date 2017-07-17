#ifndef METHODS_H
#define METHODS_H

#define PORT 58673
#define BUFFER_LENGTH 256
#define STORE_SIZE 1024
#define KEY_LENGTH 64
#define MAX_CMD 3

#define SEM_KEY 1337L

struct KeyValue {
  char key[KEY_LENGTH];
  char value[KEY_LENGTH];
};

int put(char* key, char* value, char* retVal, struct KeyValue* res);
int get(char* key, char* value, struct KeyValue* res);
int del(char* key, char* retVal, struct KeyValue* res);

#endif
