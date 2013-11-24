#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  const int bufSize = 256;
  char* buffer[bufSize];
  int fd = open("history.txt", O_RDONLY);
  while(read(fd,buffer,bufSize) > 0){
    printf(1,"%s",buffer);
  }
  close(fd);
  //退出程序
  exit();
}

