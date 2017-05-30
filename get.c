#include <stdio.h>
#include <string.h>

#include "methods.h"

int get(char* key, char* value, struct KeyValue* res) {
	int i;
	for(i = 0;i < STORE_SIZE;i++) {
		if(!strcmp(key, res[i].key)) {
			strcpy(value, res[i].value);
			return 1;
		}
	}
	return -1;
}
