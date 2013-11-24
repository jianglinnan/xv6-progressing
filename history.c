#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  const int bufSize = 256;
  char* buffer[bufSize];
  int fd = open("/.bash_history", O_RDONLY);
  if(fd == -1)
    exit();
  while(read(fd,buffer,bufSize) > 0){
    printf(1,"%s",buffer);
  }
  close(fd);
  //退出程序
  exit();
}

