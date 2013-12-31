#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "defs_struct.h"
#define MAX_NAME_LINGTH 16

int main(int argc, char *argv[])
{
	int fd = open("bat.c", O_WRONLY|O_CREATE);
	printf(fd, "int main()\n");
	printf(fd, "{\n");
	printf(fd, "  int i;\n");
	printf(fd, "  system(\"mkdir test\");\n");
	printf(fd, "  for(i=1;i<=5;i++)\n");
	printf(fd, "  {\n");
	printf(fd, "    string str=\"touch \";\n");
	printf(fd, "    system(str+i);\n");
	printf(fd, "  }\n");
	printf(fd, "  return 0;\n");
	printf(fd, "}\n");
	close(fd);
	exit();
}

