#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  //一个简单的输出语句
  //这句执行成功
  printf(1, "hongdashen xihuan hehe\n");
  //洪宇： 
  //创建一个文件，写入一些东西
  //jln
  //实测文件创建不成功，没有生成该文件
  /*int fd = open("/tmpfile/honghehe.txt", O_CREATE|O_WRONLY);
  printf(fd, "hongdashen xihuan hehe\n");
  close(fd);*/
  //退出程序
  exit();
}

