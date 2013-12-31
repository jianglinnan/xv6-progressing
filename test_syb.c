#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
int
main(int argc, char *argv[])
{
	// fd参见echo.c
	// usertest.c中的stdout定义为1
	int fd = open("makefile", O_WRONLY|O_CREATE);
	printf(fd, "%s\n", "sh = script");
	printf(fd, "%s\n", "ec = echo");
	printf(fd, "%s\n", "succ = success");
	printf(fd, "%s\n", "all: e.c f.c");
	printf(fd, "%s\n", "	$(ec) $(succ)");
	printf(fd, "%s\n", "e.c : a.c b.c");
	printf(fd, "%s\n", "	$(sh) a.c");
	printf(fd, "%s\n", "f.c : c.c d.c");
	printf(fd, "%s\n", "	$(sh) c.c");
	printf(fd, "%s\n", "clean:");
	printf(fd, "%s\n", "	rm a.c b.c c.c d.c e.c f.c");
	close(fd);
	exit();
}

