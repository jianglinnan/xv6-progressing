#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  const int bufSize = 256;
  char buffer[bufSize];
  int fd = open("/.bash_history", O_RDONLY);
  if(fd == -1)
    exit();
  while(read(fd,buffer,bufSize) > 0){
    printf(1,"%s",buffer);
    /*
    int i = 0;
    int x = strlen(buffer);
    for(i = 0; i < x; i++){
      if(buffer[i] == '\r')
        printf(1,"I am an end\n");
      if(buffer[i] == '\r')
        printf(1,"I am an enter\n");       
      if(buffer[i] == '\n')
        printf(1,"I am an new line\n");
    }*/
    memset(buffer,0,bufSize);
  }
  close(fd);
  //退出程序
  exit();
}

