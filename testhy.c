#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  //一个简单的输出语句
  printf(1, "hongyu:hello,xv6\n");
  //创建一个文件，写入一些东西
  int fd = open("hy-hello-xv6", O_CREATE|O_WRONLY);
  printf(fd, "hongyu:hello,xv6\n");
  close(fd);
  //退出程序
  exit();
}

