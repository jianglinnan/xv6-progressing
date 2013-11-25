#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define BUF_SIZE 256

int main(int argc, char *argv[])
{
	//判断参数
	if (argc == 1 || argc == 2)
	{
		printf(1, "please input the command as [cp src_file dest_file]\n");
		exit();
	}
	
	//打开源文件
	int fd_src = open(argv[1], O_RDONLY);
	if (fd_src == -1)
	{
		printf(1, "open source file failed\n");
		exit();
	}
	
	//打开目标文件
	int fd_dest = open(argv[2], O_WRONLY|O_CREATE);
	if (fd_dest == -1)
	{
		printf(1, "create dest file failed\n");
		exit();
	}
	
	//复制文件
	char buf[BUF_SIZE] = {};
	int len = 0;
	while((len = read(fd_src, buf, BUF_SIZE)) > 0)
		write(fd_dest, buf, len);
	
	//关闭文件和程序
	close(fd_src);
	close(fd_dest);
	exit();
}
