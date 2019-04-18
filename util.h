
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void error(const char *msg);
int expand_buffer(void **buffer, int size, int scale);
void remove_substr(char *str, const char *toremove);
char *get_substr(char *paragraph, char *begkey, char *endkey, int remove);
char *make_superstr(int na, int nb, char *sa, char *sb);