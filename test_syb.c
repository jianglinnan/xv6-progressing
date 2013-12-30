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
	printf(fd, "%s\n", "md = mkdir");
	printf(fd, "%s\n", "t1 = test1.o");
	printf(fd, "%s\n", "all: test1.o test2.o test3.o");
	printf(fd, "%s\n", "	$(md) all");
	printf(fd, "%s\n", "test1.o : test2.o");
	printf(fd, "%s\n", "	$(md) $(t1)");
	printf(fd, "%s\n", "t2 = test2.o");
	printf(fd, "\n");
	printf(fd, "%s\n", "test2.o : test3.o");
	printf(fd, "%s\n", "	mkdir $(t2)");
	printf(fd, "%s\n", "test3.o :");
	printf(fd, "%s\n", "	$(md) test3.o");
	printf(fd, "%s\n", "clean:");
	printf(fd, "%s\n", "	rm $(t1) $(t2) test3.o all");
	//printf(fd, "%s\n", "happy = asdfasdflk asdfa efrer");
	//printf(fd, "%s\n", "sad = lkasdlfajdslfkads d ew q q/q/ d   ");
	//printf(fd, "%s\n", "time = ak3 1 3 r re w we w    ");
	close(fd);
	exit();
}