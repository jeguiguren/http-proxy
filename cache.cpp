#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "util.h" 
#include <time.h>

#define CACHE_SIZE 10
#define DEFAULT_MAX_AGE 3600 // 1 hr



cache new_cache() {
	cache c = malloc(sizeof(struct cache));
	if (!c)
		error("Could not Initialize Cache");
	for (int i = 0; i < CACHE_SIZE; i++) {
		c->contents[i] = NULL;
	}
	return c;
}

int get_max_age(char *message) {
	char *key = "max-age=";
	char *max_age = get_substr(message, key, "\r\n", strlen(key));
	if (!max_age)
		return DEFAULT_MAX_AGE;
	//fprintf(stdout, "%s\n", max_age);
	return atoi(max_age);
}


content message_to_content(int port, char *request, char *response, int size) {
	content cont = malloc(sizeof(struct content));
	cont->birth = (int) time(0);
	cont->max_age = get_max_age(response);
	cont->accesed = cont->birth;
	cont->size = size;
	cont->port = port;
	cont->request = request;
	cont->response = response;
	//fprintf(stdout,"Storing message with life %d, and birth %d\n", cont->max_age, cont->birth);
	return cont;
}


int is_stale(content cont) {
	if (cont) {
		int age = ((int) time(0)) - cont->birth;
		return age > cont->max_age;
	}
	return 1;
}


content find_content(cache a_cache, int port, char *request) {
	content cont = NULL;
	for (int i = 0; i < CACHE_SIZE; i ++) {
		cont = a_cache->contents[i];
		if (!is_stale(cont)) {
			if (!strcmp(request, cont->request) && port == cont->port) {
				//fprintf(stdout, "Resource FOUND! %s in %d\n", request, i);
				return cont;
			}
		}
	}
	return NULL;
}


int cache_fetch(cache a_cache, int port, char *request, char **response)
{
	//fprintf(stdout, "Looking for %s\n", request);
	content cont = find_content(a_cache, port, request);
	if (!cont)
		return -1;
	cont->accesed = (int) time(0);
	int age = (int) time(0) - cont->birth;
	char sage[15];
	sprintf(sage, "Age: %d\r\n", age);

	//fprintf(stdout, "%d\n", age);
	//fprintf(stdout, "%ld\n", strlen(sage));
	char *res = make_superstr(strlen(sage), cont->size, sage, cont->response);
	*response = res;
	return cont->size + strlen(sage);
}


int find_spot(cache a_cache) {
	int oldest = 0;
	for (int i = 0; i < CACHE_SIZE; i ++) {
		content cont = a_cache->contents[i];
		if (!cont) //Empty
			return i;
		if (is_stale(cont))
			return i;
		if (cont->accesed < a_cache->contents[oldest]->accesed) //least recently accesed
			oldest = i;
	}
	return oldest;
}


void cache_store(cache a_cache, int port, char *request, char *response, int size)
{
	content cont = message_to_content(port, request, response, size);
	int spot = find_spot(a_cache);
	if (a_cache->contents[spot]) {
		free(a_cache->contents[spot]->request);
		free(a_cache->contents[spot]->response);
		free(a_cache->contents[spot]);
	}
	a_cache->contents[spot] = cont; 
	//fprintf(stdout, "\n\nResource Cached in %d: %s\n", spot, cont->request);
}


