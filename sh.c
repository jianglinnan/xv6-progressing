// Shell命令

#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "stat.h"
#include "fs.h"
#include "defs_struct.h"

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10

struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd {
  int type;
  struct cmd *cmd;
  char *file;
  char *efile;
  int mode;
  int fd;
};

struct pipecmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd {
  int type;
  struct cmd *cmd;
};

int fork1(void);  // Fork but panics on failure.
void panic(char*);
struct cmd *parsecmd(char*);
void recordHistory(char* commd);
void getHistory(struct HistoryStruct *hs);
void getExecutedCmd();
struct HistoryStruct hs;
struct ExecutedCmd ec;
char* fmtname(char* path);
void fixcmd(char* errorcmd);
char** getcmdlist(char* path, int* listlen);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2];
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    exit();
  
  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC:
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit();
    exec(ecmd->argv[0], ecmd->argv);
    printf(2, "exec %s failed\n", ecmd->argv[0]);
    fixcmd(ecmd->argv[0]);
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    close(rcmd->fd);
    if(open(rcmd->file, rcmd->mode) < 0){
      printf(2, "open %s failed\n", rcmd->file);
      exit();
    }
    runcmd(rcmd->cmd);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    if(fork1() == 0)
      runcmd(lcmd->left);
    wait();
    runcmd(lcmd->right);
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    if(fork1() == 0){
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait();
    wait();
    break;
    
  case BACK:
    bcmd = (struct backcmd*)cmd;
    if(fork1() == 0)
      runcmd(bcmd->cmd);
    break;
  }
  exit();
}


//get cmd from console
//the input result is in buf
int
getcmd(char *buf, int nbuf)
{
  printf(2, "$ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}



int
main(void)
{
  static char buf[100];
  int fd;
  getHistory(&hs);
  setHistory(&hs);
  getExecutedCmd();
  setExeCmd(&ec);
  setProgramStatus(SHELL);
  // Assumes three file descriptors open.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(strcmp(buf,"ESC\n") == 0)
      exit();
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Clumsy but will have to do for now.
      // Chdir has no effect on the parent if run in the child.
      recordHistory(buf);
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        printf(2, "cannot cd %s\n", buf+3);
      getExecutedCmd();
      setExeCmd(&ec);
      continue;
    }
    if(fork1() == 0){
      runcmd(parsecmd(buf));
    }
    recordHistory(buf);
    memset(&hs,0,sizeof(struct HistoryStruct));
    getHistory(&hs);
    setHistory(&hs);
    wait();
  }
  exit();
}

void
panic(char *s)
{
  printf(2, "%s\n", s);
  exit();
}

int
fork1(void)
{
  int pid;
  
  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

//PAGEBREAK!
// Constructors

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->efile = efile;
  cmd->mode = mode;
  cmd->fd = fd;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd*)cmd;
}
//PAGEBREAK!
// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if(*s == '>'){
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;
  
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    printf(2, "leftovers: %s\n", s);
    panic("syntax");
  }
  nulterminate(cmd);
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parsepipe(ps, es);
  while(peek(ps, es, "&")){
    gettoken(ps, es, 0, 0);
    cmd = backcmd(cmd);
  }
  if(peek(ps, es, ";")){
    gettoken(ps, es, 0, 0);
    cmd = listcmd(cmd, parseline(ps, es));
  }
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a')
      panic("missing file for redirection");
    switch(tok){
    case '<':
      cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
      break;
    case '>':
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    case '+':  // >>
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    }
  }
  return cmd;
}

struct cmd*
parseblock(char **ps, char *es)
{
  struct cmd *cmd;

  if(!peek(ps, es, "("))
    panic("parseblock");
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);
  if(!peek(ps, es, ")"))
    panic("syntax - missing )");
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;
  
  if(peek(ps, es, "("))
    return parseblock(ps, es);

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|)&;")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a')
      panic("syntax");
    cmd->argv[argc] = q;
    cmd->eargv[argc] = eq;
    argc++;
    if(argc >= MAXARGS)
      panic("too many args");
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}

// NUL-terminate all the counted strings.
struct cmd*
nulterminate(struct cmd *cmd)
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    return 0;
  
  switch(cmd->type){
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    for(i=0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    nulterminate(rcmd->cmd);
    *rcmd->efile = 0;
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    nulterminate(pcmd->left);
    nulterminate(pcmd->right);
    break;
    
  case LIST:
    lcmd = (struct listcmd*)cmd;
    nulterminate(lcmd->left);
    nulterminate(lcmd->right);
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}

void itoa(char* s,int n){
  int i = 0;
  int len = 0;
  while(n % 10 != 0){
    s[len] = n % 10 + '0';
    n = n / 10; 
    len++;
  }
  for(i = 0; i < len/2; i++){
    char tmp = s[i];
    s[i] = s[len - 1 - i];
    s[len - 1 - i] = tmp;
  }
  s[len] = '\0';
}

void
recordHistory(char* commd){
    const int bufSize = 256;
    char buf[bufSize];
    int fd = open("/.bash_history",O_RDWR|O_CREATE);
    while(read(fd,buf,bufSize)){
      memset(buf,0,bufSize);
    }
    write(fd,commd,strlen(commd));
    close(fd);
}

void getHistory(struct HistoryStruct* hs){
  int fd = open("/.bash_history",O_RDONLY|O_CREATE);
  const int bufSize = 256;
  char buf[bufSize];
  int locs[bufSize];
  char pre[bufSize];
  char sub[bufSize];
  int j;
  memset(pre,'\0',bufSize);

  //num of histories
  int nums = 0;
  int len;
  while((len = read(fd,buf,bufSize)) > 0){
    //num of '\n'
    int nl = 0;
    memset(locs,0,bufSize);
    //record the location of every '\n'
    for(j = 0; j < strlen(buf); j++){
      if(buf[j] == '\n')
      {
        locs[nl] = j;
        nl++;
      }
    }
    if(nl >= 1){
      memset(hs->history[nums % H_ITEMS],0,H_ITEMS);
      memset(sub,0,bufSize);
      strcat(hs->history[nums % H_ITEMS],pre,substring(sub,buf,0,locs[0]));
      nums++;
    }
    for(j = 1; j <= nl - 1; j++){
      memset(hs->history[nums % H_ITEMS],0,H_ITEMS);
      memset(sub,0,bufSize);
      strcpy(hs->history[nums % H_ITEMS],substring(sub,buf,locs[j-1] + 1,locs[j]));
      nums++;
    }
    if(len == bufSize && locs[nl-1] < len - 1){
      memset(pre,0,bufSize);
      memset(sub,0,bufSize);
      strcpy(pre,substring(sub,buf,locs[nl-1] + 1,len));
      memset(buf,0,bufSize);
    }
  }
  close(fd);
  hs->len = nums;
  hs->start = (nums - 1) % H_ITEMS;
  hs->current = hs->start;
  return;
}

#define STATIC_CMD_LEN 12

void getExecutedCmd(){

  char staticCommands[STATIC_CMD_LEN][16] = {
    "/ls",
    "/mkdir",
    "/cp",
    "/mv",
    "/rename",
    "/rm",
    "/cat",
    "/history",
    "/editor",
    "/uptime",
    "/help",
    "/script"
  };

  int length = 0;
  int flag = 0;
  char** cmd = getcmdlist(".",&length);
  if(length >= MAX_EXECMD)
    length = MAX_EXECMD;
  int i = 0;
  ec.len = length;
  for(i = 0; i < length; i++){
    if(strcmp(cmd[i],"ls") == 0){
      flag = 1;
    }
    strcpy(ec.commands[i],cmd[i]);
  }
  if(flag == 0){
    for(i = length; i < length + STATIC_CMD_LEN; i++){
      strcpy(ec.commands[i],staticCommands[i - length]);
    }
    ec.len += STATIC_CMD_LEN;
  }
}


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
  cmdall = malloc(sizeof(char*)*MAX_EXECMD);
  int i = 0;
  while(1){
    cmdall[i] = malloc(sizeof(char)*MAX_EXECMD);
    i ++;
    if(i == MAX_EXECMD)
      break;
  }

  int cmdflag = 0;
  for (i = 0;i < MAX_EXECMD;i ++)
    memset(cmdall[i], 0, MAX_EXECMD);
  
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
      if(st.type == 2){
        strcpy(cmdall[cmdflag], fmtname(buf));
        int j = 0;
        for (j = strlen(cmdall[cmdflag]) - 1;j >= 0;j --){
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

int
threeintmin(int x1, int x2, int x3){
  if ((x1 <= x2) && (x1 <= x3))
    return x1;
  else if ((x2 <= x1) && (x2 <= x3))
    return x2;
  else
    return x3;
}

int
stringdistance(char* s1, char* s2)
{
  int len1 = (int)strlen(s1);
  int len2 = (int)strlen(s2);

  int **dp;
  dp = malloc(sizeof(int*)*len1);
  int i = 0;
  for (i = 0;i < len1;i ++){
    dp[i] = malloc(sizeof(int)*len2);
  }
  for (i = 0;i < len1;i ++){
    dp[i][0] = i;
  }
  for (i = 0;i < len2;i ++){
    dp[0][i] = i;
  }

  dp[0][0] = 0;

  int j = 0;
  for (i = 1;i < len1;i ++){
    for (j = 1;j < len2;j ++){
      if (s1[i - 1] == s2[j - 1])
        dp[i][j] = dp[i - 1][j - 1];
      else
        dp[i][j] = threeintmin(dp[i][j - 1], dp[i - 1][j], dp[i - 1][j - 1]) + 1;
    }
  }

  int ans = dp[len1 - 1][len2 - 1];
  free(dp);
  return ans;
}

void
fixcmd(char *errorcmd)
{
  int cmdlistlen;
  char **cmdlist;
  cmdlist = getcmdlist(".", &cmdlistlen);
  int* cmdlistflag;
  int* cmdlistdis;
  cmdlistflag = malloc(sizeof(int)*cmdlistlen);
  cmdlistdis = malloc(sizeof(int)*cmdlistlen);
  int i;
  for (i = 0;i < cmdlistlen;i ++){
    cmdlistflag[i] = i;
  }
  for (i = 0;i < cmdlistlen;i ++){
    cmdlistdis[i] = stringdistance(errorcmd, cmdlist[i]);
  }
  int j, k;
  for (i = 0;i < cmdlistlen - 1;i ++){
    for (j = 0;j < cmdlistlen - 1 - i;j ++)
      if (cmdlistdis[j] > cmdlistdis[j + 1]){
        k = cmdlistdis[j];
        cmdlistdis[j] = cmdlistdis[j + 1];
        cmdlistdis[j + 1] = k;
        k = cmdlistflag[j];
        cmdlistflag[j] = cmdlistflag[j + 1];
        cmdlistflag[j + 1] = k;
      }
  }

  printf(1, "%s\n", "Your cmd is illegal!");
  printf(1, "%s\n", "Maybe you want to input these commands...");
  for (i = 0;i < 4;i ++){
    printf(1, "%s\n", cmdlist[cmdlistflag[i]]);
  }

}
