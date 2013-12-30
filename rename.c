#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[])
{
	//判断参数
	if (argc == 1 || argc == 2)
	{
		printf(1, "please input the command as [cp src_file dest_file]\n");
		exit();
	}
	int result = link(argv[1], argv[2]);
	//链接成功
	if (result == 0)
		unlink(argv[1]);
	else
		printf(1, "rename failed, maybe the file is a directory.\n");
	exit();
}

