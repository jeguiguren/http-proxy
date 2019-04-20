#include "util.h"

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

