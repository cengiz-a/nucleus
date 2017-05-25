#include <stdio.h>
#include "methods.h"

int put(char* key, char* value, struct KeyValue* res) {
	strcpy(res[0].key, key);
	strcpy(res[0].value, value);
	return 0;
}
