#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
	int fd = open("fib.c", O_WRONLY|O_CREATE);
	//一个简单的输出语句
	printf(fd, "int f(int n)\n");
	printf(fd, "{\n");
	printf(fd, " if(n==1)\n");
	printf(fd, " {\n");
	printf(fd, "  return 1;\n");
	printf(fd, " }\n");
	printf(fd, " if(n==2)\n");
	printf(fd, " {\n");
	printf(fd, "  return 1;\n");
	printf(fd, " }\n");
	printf(fd, " if(n>=3)\n");
	printf(fd, " {\n");
	printf(fd, "  int a=f(n-1);\n");
	printf(fd, "  int b=f(n-2);\n");
	printf(fd, "  return a+b;\n");
	printf(fd, " }\n");
	printf(fd, "}\n");
	printf(fd, "int main()\n");
	printf(fd, "{\n");
	printf(fd, " int i=1;\n");
	printf(fd, " int res=0;\n");
	printf(fd, " for(i=1;i<=4;i++)\n");
	printf(fd, " {\n");
	printf(fd, "  res=f(i);\n");
	printf(fd, "  system(\"echo \"+res);\n");
	printf(fd, " }\n");
	printf(fd, " return 0;\n");
	printf(fd, "}\n");
	close(fd);
	//退出程序
	exit();
}

