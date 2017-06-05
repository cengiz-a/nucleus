#include <stdio.h>
#include "methods.h"
#include <string.h>

int put(char* key, char* value, char* retVal, struct KeyValue* res) {
  int i;
	for(i = 0;i < STORE_SIZE;i++) {
        if(res[i].key[0] == '\0' || !strcmp(res[i].key, key)) {
          strcpy(retVal, res[i].value);
          strcpy(res[i].key, key);
          strcpy(res[i].value, value);
          return 1;
        }
	}

	return -1;
}
