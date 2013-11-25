#include "types.h"
#include "stat.h"
#include "user.h"

char buf[512];

void
cat(int fd)
{
  int n;

  while((n = read(fd, buf, sizeof(buf))) > 0)
    write(1, buf, n);
  if(n < 0){
    printf(1, "cat: read error\n");
    exit();
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;

  if(argc <= 1){
    cat(0);
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

  // 处理“-”为标准输入
  for(i = 1; i < argc; i++){
    if(strcmp(argv[i], "-") == 0){
      cat(0);
      exit();
    }
  }

  for(i = 1; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf(1, "cat: cannot open %s\n", argv[i]);
      exit();
    }
    cat(fd);
    close(fd);
  }
  exit();
}
