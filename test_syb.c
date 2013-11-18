#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
	// fd参见echo.c
	// usertest.c中的stdout定义为1
	printf(1, "Sometimes hard...\n");
	exit();
}