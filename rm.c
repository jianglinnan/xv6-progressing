#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

char*
fmtname(char* path)
{
  static char buf[DIRSIZ+1];
  char *p;
  
  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  
  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

// 获取path路径下的所有命令
char**
getcmdlist(char* path, int* listlen)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  char **cmdall;
  cmdall = malloc(sizeof(char*)*30);
  int i = 0;
  while(1){
    cmdall[i] = malloc(sizeof(char)*30);
    i ++;
    if(i == 30)
      break;
  }

  int cmdflag = 0;
  for (i = 0;i < 30;i ++)
    memset(cmdall[i], 0, 30);
  
  if((fd = open(path, 0)) < 0){
    printf(2, "getcmdlist: cannot open %s\n", path);
  }
  
  if(fstat(fd, &st) < 0){
    printf(2, "getcmdlist: cannot stat %s\n", path);
    close(fd);
  }
  
  switch(st.type){
  case T_FILE:
    printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
    break;
  
  case T_DIR:

    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "getcmdlist: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "getcmdlist: cannot stat %s\n", buf);
        continue;
      }
      if(st.type == 1 || st.type == 2){
        strcpy(cmdall[cmdflag], fmtname(buf));
        int j = 0;
        for (j = strlen(cmdall[cmdflag]) - 1;j >= 0; j--){
          if (cmdall[cmdflag][j] == ' ')
            cmdall[cmdflag][j] = 0;
          else
            break;
        }
        cmdflag++;
      }
    }
    break;
  }
  close(fd);
  *listlen = cmdflag;

  return (char**)cmdall;
}


 
int recurRM(char* path){
  int j,len;
  char** cmd;
  int result = 0;
  if(chdir(path) < 0){
      if(unlink(path) < 0){
        printf(1, "rm: %s file failed to delete\n", path);
        return -1;
      }
      return 0; 
  }
  cmd = getcmdlist(".",&len);
  for(j = 0; j < len; j++){
    if(strcmp(cmd[j],".") == 0 || strcmp(cmd[j],"..") == 0)
      continue;
    if(recurRM(cmd[j]) < 0){
      printf(1, "rm: %s/%s failed to delete\n", path,cmd[j]);
      result = -1;
      continue;
    } 
  }
  free(cmd);
  chdir("../");
  if(unlink(path) < 0){
    printf(1, "rm: %s diretory failed to delete\n", path);
    result = -1;
  }
  return result;
}

int
main(int argc, char *argv[])
{
  int i;
  //char** cmd;
  if(argc < 2){
    printf(2, "Usage: rm files...\n");
    exit();
  }

  for(i = 1; i < argc; i++){
    if(recurRM(argv[i]) < 0){
      printf(1, "rm: %s failed to delete\n", argv[i]);
      continue;
    }
    /*
    if(chdir(argv[i]) < 0){
      if(unlink(argv[i]) < 0){
        printf(2, "rm: %s failed to delete\n", argv[i]);
        continue;
      }
      continue; 
    }
    cmd = getcmdlist(".",&len);
    for(j = 0; j < len; j++){
      if(unlink(cmd[j]) < 0){
        printf(2, "rm: %s/%s failed to delete\n", argv[i],cmd[j]);
        break;
      } 
    }
    free(cmd);
    chdir("../");
    if(unlink(argv[i]) < 0){
      printf(2, "rm: %s diretory failed to delete\n", argv[i]);
      continue;
    }*/
  }

  exit();
}