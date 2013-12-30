#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  	//一个简单的输出语句
  	int fd = open("makefiletest", O_WRONLY|O_CREATE);
 	printf(fd, "%s\n", "edit : main.o kbd.o command.o display.o insert.o search.o files.o utils.o");
	printf(fd, "%s\n", "	cc -o edit main.o kbd.o command.o display.o insert.o search.o files.o utils.o");
	printf(fd, "\n");
	printf(fd, "%s\n", "main.o : main.c defs.h");
	printf(fd, "%s\n", "	cc -c main.c");
	printf(fd, "%s\n", "kbd.o : kbd.c defs.h command.h");
	printf(fd, "%s\n", "	cc -c kbd.c");
	printf(fd, "%s\n", "command.o : command.c defs.h command.h");
	printf(fd, "%s\n", "	cc -c command.c");
	printf(fd, "%s\n", "display.o : display.c defs.h buffer.h");
	printf(fd, "%s\n", "	cc -c display.c");
	printf(fd, "%s\n", "insert.o : insert.c defs.h buffer.h");
	printf(fd, "%s\n", "	cc -c insert.c");
	printf(fd, "%s\n", "search.o : search.c defs.h buffer.h");
	printf(fd, "%s\n", "	cc -c search.c");
	printf(fd, "%s\n", "files.o : files.c defs.h buffer.h command.h");
	printf(fd, "%s\n", "	cc -c files.c");
	printf(fd, "%s\n", "utils.o : utils.c defs.h");
	printf(fd, "%s\n", "	");
	printf(fd, "%s\n", "clean :");
	printf(fd, "%s\n", "	rm edit main.o kbd.o command.o display.o insert.o search.o files.o utils.o");
	close(fd);
  //退出程序
  exit();
}

