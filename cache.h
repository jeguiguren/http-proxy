#include <stdio.h>
#include <stdlib.h>
#include <string.h>



typedef struct content {
	int birth;
	int max_age;
	int accesed;
	int size;
	int port;
	char *request;
	char *response;
} *content;

typedef struct cache {
	content contents[CACHE_SIZE];
} *cache;

cache new_cache();
int cache_fetch(cache a_cache, int port, char *request, char **response);
void cache_store(cache a_cache, int port, char *request, char *response, int size);

