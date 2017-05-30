#include <stdio.h>
#include <string.h>

#include "methods.h"

int del(char* key, struct KeyValue* res) {
  int i;
	for(i = 0;i < STORE_SIZE;i++) {
		if(!strcmp(key, res[i].key)) {
			res[i].key[0] = '\0';
      res[i].value[0] = '\0';
      printf("RES: %s\n", res[i].value);
			return 1;
		}
	}
	return -1;
}
