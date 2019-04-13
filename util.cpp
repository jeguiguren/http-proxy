
#include "util.h"

char *make_superstr(int na, int nb, char *sa, char *sb) {
	
	void *buffer = malloc(na + nb);

	char *rest = strstr(sb, "\r\n");
	int firstSize = rest - sb;
	fprintf(stderr, "%d\n", firstSize);

	memcpy(buffer, sb, firstSize);
	memcpy(buffer + firstSize, "\r\n", 2);
	memcpy(buffer + firstSize + 2, sa, na);
	memcpy(buffer + firstSize + 2 + na, rest + 2, nb - firstSize - 2);
	return buffer;
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}


int expand_buffer(void **buffer, int size, int scale) {
	void *new_buffer = malloc(size + scale); 
	memcpy(new_buffer, (*buffer), size); 
	free(*buffer); 
	(*buffer) = new_buffer;
	return size + scale;
}


char *get_substr(char *paragraph, char *begkey, char *endkey, int remove) {
	int substr_size;
	char *beg = strstr(paragraph, begkey);
	if (!beg)
		return NULL;
	if (endkey == NULL){
		int l = strlen(paragraph);
		substr_size = l - (1 + beg - paragraph);;
	} else{
		char *end = strstr(beg, endkey);
		substr_size = end - beg - remove;
		if (!end)
			return NULL;
	}

	//fprintf(stdout, "Allocating %d bytes for '%s'\n", substr_size, begkey);

	//fprintf(stdout, "\n---\nENdkey Length: %ld, Substr Length: %ld\n", strlen(endkey), end-beg);
	char *buffer = malloc(substr_size + 1);
	if (!buffer)
		error("Could not allocate memory");
	memset(buffer, 0, substr_size);
	memcpy(buffer, beg + remove, substr_size);
	buffer[substr_size] = '\0';
	
	//fprintf(stdout, "Ret: %s, size: %ld\n", buffer, strlen(buffer));
	return buffer;
}