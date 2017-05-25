#ifndef METHODS_H
#define METHODS_H

struct KeyValue {
  char key[64];
  char value[64];
};

int put(char* key, char* value, struct KeyValue* res);
int get(char* key, struct KeyValue* res);
int del(char* key, struct KeyValue* res);

#endif
