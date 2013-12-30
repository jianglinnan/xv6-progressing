#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf[512];

void
cat(int fd,int dst)
{
  int n;
  while((n = read(fd,buf,sizeof(buf))) > 0){
    write(dst, buf, n);
    memset(buf,0,sizeof(buf));
  }

  if(n < 0){
    printf(1, "cat: read error\n");
    exit();
  }
}

int
main(int argc, char *argv[])
{
  int fd,fr,i;
  if(argc <= 1){
    cat(0,1);
    exit();
  }

  // 处理--help参数
  for(i = 1; i < argc; i++){
    if(strcmp(argv[i], "--help") == 0){
      printf(1, "SYNOPSIS\n");
      printf(1, "    cat [FILE]...\n");
      printf(1, "DESCRIPTION\n");
      printf(1, "    Concatenate files, or standard input, to standard output.\n\n");
      printf(1, "    --help              display this help and exit\n\n");
      printf(1, "    With no FILE, or when FILE is -, read standard input.\n");
      printf(1, "EXAMPLES\n");
      printf(1, "    cat f               Output f's contents.\n");
      printf(1, "    cat -               Output standard input's contents.\n");
      printf(1, "    cat                 Output standard input's contents.\n");
      exit();
    }
  }

  int loc = 0;
  for(i = 1; i < argc; i++){
    if(strcmp(argv[i], ">") == 0 && i + 2 == argc)
    {
      loc = i;
      break;
    }
  }
  if(strcmp(argv[1], "-") == 0 && argc == 3)
    loc = 1;

  if(loc == 1){
    char buf[512];
    fd = open(argv[2],O_CREATE|O_WRONLY);
    while(1){
      gets(buf,512);
      if(strcmp(buf,"/exit\n") == 0 || strcmp(buf,"ESC\n") == 0){
        close(fd);
        exit();
      }
      write(fd, buf, strlen(buf));
      memset(buf,0,512);
    }
  }

  if(loc > 1){
    fd = open(argv[argc-1],O_CREATE|O_WRONLY);
    for(i = 1; i < loc; i++){
      if((fr = open(argv[i],O_RDONLY)) < 0){
        printf(1, "cat: cannot open %s\n", argv[i]);
        exit();
      }
      cat(fr,fd);
      close(fr);
    }
    close(fd);
  }

  else{
    for(i = 1; i < argc; i++){
      if((fd = open(argv[i], 0)) < 0){
        printf(1, "cat: cannot open %s\n", argv[i]);
        exit();
      }
      cat(fd,1);
      close(fd);
    }   
  }
  exit();
}
