#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
	//一个简单的输出语句
	printf(1, "int f(int n)\n");
	printf(1, "{\n");
	printf(1, " if(n==1)\n");
	printf(1, " {\n");
	printf(1, "  return 1;\n");
	printf(1, " }\n");
	printf(1, " if(n==2)\n");
	printf(1, " {\n");
	printf(1, "  return 1;\n");
	printf(1, " }\n");
	printf(1, " if(n>=3)\n");
	printf(1, " {\n");
	printf(1, "  int a=f(n-1);\n");
	printf(1, "  int b=f(n-2);\n");
	printf(1, "  return a+b;\n");
	printf(1, " }\n");
	printf(1, "}\n");
	printf(1, "int main()\n");
	printf(1, "{\n");
	printf(1, " int i=1;\n");
	printf(1, " int res=0;\n");
	printf(1, " for(i=1;i<=4;i++)\n");
	printf(1, " {\n");
	printf(1, "  res=f(i);\n");
	printf(1, "  system(\"echo \"+res);\n");
	printf(1, " }\n");
	printf(1, " return 0;\n");
	printf(1, "}\n");
	//退出程序
	exit();
}

