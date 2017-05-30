#ifndef METHODS_H
#define METHODS_H

#define PORT 4711
#define BUFFER_LENGTH 256
#define STORE_SIZE 1024
#define KEY_LENGTH 64
#define MAX_CMD 3

struct KeyValue {
  char key[KEY_LENGTH];
  char value[KEY_LENGTH];
};

int put(char* key, char* value, struct KeyValue* res);
int get(char* key, char* value, struct KeyValue* res);
int del(char* key, struct KeyValue* res);

#endif
