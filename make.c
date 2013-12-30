#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"
#include "defs_struct.h"
#define BUF_SIZE 256
#define MAX_LINE_NUMBER 512
#define MAX_LINE_LENGTH 256
#define SUBTREE_NUMBER 10
//#define O_RDWR 4
#define NULL 0
#define MAX_NAME_LINGTH 16

int split(char *src, char pattern, char *result[]);
char* strcat_n(char* dest, char* src, int len);
char* fmtname(char* path);
void fixcmd(char* errorcmd);
char** getcmdlist(char* path, int* listlen);
void runcomand(char* cmd);
char* combinetwostring(char* src, int st, int ed, char* dest);

typedef struct stringnode
{
	char* data;
	struct stringnode* next;
}stringlist;

typedef struct binode
{
	char* name;

	char** orders;
	int orders_num;

	char** depends;
	int depends_num;

	struct binode* next;
	int subtree_num;
	struct binode** subtree;
}NODE;

typedef struct pinode
{
	char* name;
	char* data;
	struct pinode* next;
}VNODE;

void makefiletree(NODE* tree);
stringlist* getsplitlist(char* data, int linepop, int* dpnum);

int
main(int argc, char *argv[])
{
	//存放根节点名称
	char* root = NULL;
	//判断用户是否指定根节点名称
	if (argc > 1)
		root = argv[1];
	//存放文件内容
	char *text[MAX_LINE_NUMBER] = {};
	text[0] = malloc(MAX_LINE_LENGTH);
	memset(text[0], 0, MAX_LINE_LENGTH);
	//存储当前最大的行号，从0开始。即若line_number == x，则从text[0]到text[x]可用
	int line_number = 0;
	//尝试打开Makefile文件
	int fd = open("makefile", O_RDWR);
	if (fd == -1)
		fd = open("makefile", O_RDWR);
	if (fd == -1)
	{
		printf(1, "Can't find makefile. Halt. \n");
		return 0;
	}
	//如果文件存在，则打开并读取里面的内容
	if (fd != -1)
	{
		char buf[BUF_SIZE] = {};
		int len = 0;
		while ((len = read(fd, buf, BUF_SIZE)) > 0)
		{
			int i = 0;
			int next = 0;
			int is_full = 0;
			while (i < len)
			{
				//拷贝"\n"之前的内容
				for (i = next; i < len && buf[i] != '\n'; i++)
					;
				strcat_n(text[line_number], buf+next, i-next);
				//必要时新建一行
				if (i < len && buf[i] == '\n')
				{
					if (line_number >= MAX_LINE_NUMBER - 1)
						is_full = 1;
					else
					{
						line_number++;
						text[line_number] = malloc(MAX_LINE_LENGTH);
						memset(text[line_number], 0, MAX_LINE_LENGTH);
					}
				}
				if (is_full == 1 || i >= len - 1)
					break;
				else
					next = i + 1;
			}
			if (is_full == 1)
				break;
		}
		close(fd);
	}

	NODE* head;
	head = malloc(sizeof(NODE));
	head -> next = NULL;

	VNODE* vhead;
	vhead = malloc(sizeof(VNODE));
	vhead -> next = NULL;

	int pop = 0;

	//处理makefile的文本内容
	//将所有依赖项借点存入head链表中
	while (pop <= line_number)
	{
		if (strlen(text[pop]) == 0)
		{
			pop ++;
			continue;
		}
		int linepop = 0;
		char charpop = text[pop][linepop];

		//分离出节点的名称
		while ((charpop != ' ') && (charpop != '=') && (charpop != ':'))
		{
			linepop ++;
			charpop = text[pop][linepop];
		}

		//将节点名称暂存在mainname中
		char* mainname;
		mainname = malloc(sizeof(char)*linepop);
		memset(mainname, 0, linepop);
		strcat_n(mainname, text[pop], linepop);
		int mainnamelen = strlen(mainname);

		//处理依赖项或赋值项
		while ((charpop == ' ') && (linepop < strlen(text[pop]) - 1))
		{
			linepop ++;
			charpop = text[pop][linepop];
		}

		if (linepop < strlen(text[pop]))
		{
			//如果后面是依赖项
			if (text[pop][linepop] == ':')
			{
				//建立一个新的节点，存储节点的名称
				NODE* newnode;
				newnode = malloc(sizeof(NODE));
				newnode -> name = malloc(sizeof(char)*mainnamelen);
				memset(newnode -> name, 0, mainnamelen);
				strcat_n(newnode -> name, mainname, mainnamelen);
				//printf(1, "%s\n", newnode -> name);

				//将节点插入节点链表中
				newnode -> next = head -> next;
				head -> next = newnode;
				linepop ++;
				int dpnum = 0;			//依赖项的个数
				stringlist* dplist;		//依赖项的字符串链表
				dplist = malloc(sizeof(stringlist));
				dplist -> next = NULL;
				if (root == NULL)
					root = head -> next -> name;
				stringlist* splitlist = getsplitlist(text[pop], linepop, &dpnum);
				dplist -> next = splitlist -> next;

				//将依赖项字符串链表存入节点中
				newnode -> depends_num = dpnum;
				newnode -> depends = malloc(sizeof(char*)*dpnum);
				int i = 0;
				for (i = 0;i < dpnum;i ++)
				{
					dplist = dplist -> next;
					newnode -> depends[i] = malloc(sizeof(char)*(strlen(dplist -> data) + 1));
					memset(newnode -> depends[i], 0, strlen(dplist -> data) + 1);
					strcat_n(newnode -> depends[i], dplist -> data, strlen(dplist -> data));
				}

				//初始化树结构
				newnode -> subtree = malloc(sizeof(NODE*) * dpnum);
				for (i = 0;i < dpnum;i ++)
				{
					newnode -> subtree[i] = NULL;
				}

				//得到所有操作语句
				//先将所有语句存在一个字符串链表中
				int ordernum = 0;
				stringlist* orderlist;		//操作语句的字符串链表
				orderlist = malloc(sizeof(stringlist));
				orderlist -> next = NULL;
				pop ++;

				while (pop <= line_number)
				{
					if ((text[pop][0] == '\t') && (strlen(text[pop]) > 1))
					{
						ordernum ++;
						int i = 1;
						int templen = 0;
						templen = strlen(text[pop]);
						char* tempstr;
						tempstr = malloc(sizeof(char)*(templen + 1));
						memset(tempstr, 0, templen + 1);
						for (i = 1;i < templen;i ++)
						{
							tempstr[i - 1] = text[pop][i];
						}
						tempstr[templen] = 0;

						for (i = 0;i < strlen(tempstr);i ++)
						{
							if ((tempstr[i] == '$') && (tempstr[i + 1] == '('))
							{
								int popst = i + 2;
								int poped = i + 2;
								while ((tempstr[poped] != ')') && (poped < strlen(tempstr)))
									poped ++;
								poped --;
								if (poped >= popst)
								{
									char* value;
									value = malloc(sizeof(char)* (poped - popst + 2));
									int j = 0;
									for (j = popst;j <= poped;j ++)
										value[j - popst] = tempstr[j];
									value[poped - popst + 1] = 0;

									VNODE* vlist = vhead -> next;
									while (vlist != NULL)
									{
										if (strcmp(vlist -> name, value) == 0)
											break;
										vlist = vlist -> next;
									}
									if (vlist != NULL)
										tempstr = combinetwostring(tempstr, i, poped + 1, vlist -> data);
									else
									{
										printf(1, "%s %s\n", "Can't find the value", value);
										exit();
									}
									i = strlen(vlist -> data) - poped + popst;
									continue;
								}
								else{
									printf(1, "%s\n", "Illegal input.");
									exit();
								}
							}
						}

						stringlist* newstring;
						newstring = malloc(sizeof(stringlist));
						newstring -> data = malloc(sizeof(char)*(strlen(tempstr) + 1));
						//for (i = 1;i < templen;i ++)
						//{
						//	newstring -> data[i - 1] = text[pop][i];
						//}
						strcat_n(newstring -> data, tempstr, strlen(tempstr));
						newstring -> next = orderlist -> next;
						orderlist -> next = newstring;
						pop ++;
					}
					else
						break;
				}

				//将字符串链表中的数据存入节点中
				newnode -> orders_num = ordernum;
				newnode -> orders = malloc(sizeof(char*)*ordernum);
				for (i = 0;i < ordernum;i ++)
				{
					orderlist = orderlist -> next;
					newnode -> orders[i] = malloc(sizeof(char)*(strlen(orderlist -> data) + 1));
					memset(newnode -> orders[i], 0, strlen(orderlist -> data) + 1);
					strcat_n(newnode -> orders[i], orderlist -> data, strlen(orderlist -> data));
				}
			}
			else if (text[pop][linepop] == '=')
			{
				//建立一个新的节点，存储节点的名称
				VNODE* newvnode;
				newvnode = malloc(sizeof(VNODE));
				newvnode -> name = malloc(sizeof(char)*mainnamelen);
				memset(newvnode -> name, 0, mainnamelen);
				strcat_n(newvnode -> name, text[pop], mainnamelen);
				newvnode -> next = vhead -> next;
				vhead -> next = newvnode;

				linepop ++;
				charpop = text[pop][linepop];
				while (charpop == ' ')
				{
					linepop ++;
					charpop = text[pop][linepop];
				}
				int linestpop = linepop;
				int poplen = strlen(text[pop]);
				int lineedpop = poplen - 1;
				charpop = text[pop][lineedpop];
				while (charpop == ' ')
				{
					lineedpop --;
					charpop = text[pop][lineedpop];
				}
				newvnode -> data = malloc(sizeof(char)*(lineedpop - linestpop + 2));
				int i = 0;
				for (i = linestpop; i <= lineedpop;i ++)
					newvnode -> data[i - linestpop] = text[pop][i];
				newvnode -> data[lineedpop - linestpop + 1] = 0;
				pop ++;
				//printf(1, "%s\n", newvnode -> data);
				continue;
			}
		}
		else
		{
			pop ++;
			continue;
		}

	}

	if (strcmp(argv[1], "clean") == 0)
	{
		NODE* findclean = head -> next;
		while (findclean != NULL)
		{
			if (strcmp(findclean -> name, "clean") == 0)
				break;
			else
				findclean = findclean -> next;
		}
		if (findclean != NULL)
		{
			int i = 0;
			for (i = 0;i < findclean -> orders_num;i ++)
			{
				printf(1, "%s\n", findclean -> orders[i]);
				runcomand(findclean -> orders[i]);
			}
			printf(1, "%s\n", "Make clean success.");
		}
		else
			printf(1, "%s\n", "Can't find clean.");
		exit();
	}

	//建立依赖关系树
	NODE* nodepop = head -> next;
	NODE* nodetemp;
	NODE* noderoot;
	while (nodepop != NULL)
	{
		nodetemp = head -> next;
		if (strcmp(nodepop -> name, root) == 0)
			noderoot = nodepop;
		while (nodetemp != NULL)
		{
			int i = 0;
			for (i = 0;i < nodepop -> depends_num;i ++)
			{
				if (strcmp(nodetemp -> name, nodepop -> depends[i]) == 0)
				{
					nodepop -> subtree[i] = nodetemp;
				}
			}
			nodetemp = nodetemp -> next;
		}
		nodepop = nodepop -> next;
	}

	//通过对树的遍历，完成make操作
	makefiletree(noderoot);
	printf(1, "%s\n", "Make success.");
	exit();

}

//合并两个字符串，将src从st到ed的部分替换为dest
char* combinetwostring(char* src, int st, int ed, char* dest)
{
	char *result;
	int resultlen = strlen(src) - (ed - st + 1) + strlen(dest) + 1;
	result = malloc(sizeof(char)*resultlen);
	memset(result, 0, resultlen);
	int i = 0;
	for (i = 0;i < st;i ++)
	{
		result[i] = src[i];
	}

	for (i = 0;i < strlen(dest);i ++)
	{
		result[st + i] = dest[i];
	}

	for (i = ed + 1;i < strlen(src);i ++)
	{
		result[i - ed - 1 + st + strlen(dest)] = src[i];
	}
	result[resultlen - 1] = 0;

	return result;
}

//将一个字符串根据空格拆分，存入链表中
stringlist* getsplitlist(char* data, int linepop, int* wordnum)
{
	char charpop;
	charpop = data[linepop];
	int dpnum = 0;

	stringlist* dplist;		//依赖项的字符串链表
	dplist = malloc(sizeof(stringlist));
	dplist -> next = NULL;
	//将所有依赖项放入字符串链表中
	while (linepop < strlen(data))
	{
		if (charpop == ' ')
		{
			linepop ++;
			charpop = data[linepop];
			continue;
		}
		int dpst = linepop;
		while ((charpop != ' ') && (linepop < (strlen(data) - 1)))
		{
			linepop ++;
			charpop = data[linepop];
		}

		if (data[linepop] != ' ')
		{
			linepop ++;
		}
		int dped = linepop;
		dpnum ++;
		stringlist* newstring;
		newstring = malloc(sizeof(stringlist));
		newstring -> data = malloc(sizeof(char) * ((dped - dpst) + 1));
		memset(newstring -> data, 0, (dped - dpst) + 1);
		int i = 0;
		for (i = dpst;i < dped; i ++)
		{
			newstring -> data[i - dpst] = data[i];
		}
		//结尾符赋值
		newstring -> data[dped - dpst] = 0;

		//将字符串插入链表的开头
		newstring -> next = dplist -> next;
		dplist -> next = newstring;
	}
	*wordnum = dpnum;
	return dplist;

}

//遍历树结构
void makefiletree(NODE* tree)
{
	int i = 0;
	//printf(1, "%s\n", tree -> name);
	for (i = 0;i < tree -> depends_num;i ++)
	{
		if (tree -> subtree[i] != NULL)
		{
			makefiletree(tree -> subtree[i]);
		}
	}

	int filelistlen = 0;
	char **filelist = getcmdlist(".", &filelistlen);

	int j = 0;
	int totalflag = 1;
	for (i = 0;i < tree -> depends_num;i ++)
	{
		int tempflag = 0;
		for (j = 0;j < filelistlen;j ++)
		{
			if (strcmp(tree -> depends[i], filelist[j]) == 0)
			{
				tempflag = 1;
				break;
			}
		}
		if (tempflag == 0)
		{
			printf(1, "%s %s.\n", "Can't find", tree -> depends[i]);
			totalflag = 0;
			exit();
		}
	}

	if (totalflag == 1)
	{
		for (i = 0;i < tree -> orders_num;i ++)
		{
			printf(1, "%s\n", tree -> orders[i]);
			runcomand(tree -> orders[i]);
		}
	}
}

//运行一条命令
void runcomand(char* cmd)
{
	char *testres[256] = {};
	split(cmd, ' ', testres);
	//printf(1, "%s\n", testres[0]);
	if (fork() == 0)
	{
		exec(testres[0], testres);
	}
	wait();
}


//拼接src的前len个字符到dest
char* strcat_n(char* dest, char* src, int len)
{
	if (len <= 0)
		return dest;
	int pos = strlen(dest);
	if (len + pos >= MAX_LINE_LENGTH)
		return dest;
	int i = 0;
	for (; i < len; i++)
		dest[i+pos] = src[i];
	dest[len+pos] = '\0';
	return dest;
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
  cmdall = malloc(sizeof(char*)*100);
  int i = 0;
  while(1){
    cmdall[i] = malloc(sizeof(char)*30);
    i ++;
    if(i == 100)
      break;
  }

  int cmdflag = 0;
  for (i = 0;i < 100;i ++)
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
      if((st.type == 2) || (st.type == 1)) {
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

//将一段内容按照某个字符分割，pattern不能是单双引号
int split(char *src, char pattern, char *result[])
{
	//标记是否处于引号中
	int single_quotes = 0;
	int double_quotes = 0;
	
	result[0] = malloc(MAX_NAME_LINGTH);
	memset(result[0], 0, MAX_NAME_LINGTH);
	int number = 0;
	int len = strlen(src);
	int position = 0;
	int i = 0;
	while (i < len)
	{
		for (; i < len && (src[i] != pattern || single_quotes || double_quotes); i++) {
			if (src[i] == '\'')
				single_quotes = single_quotes ^ 1;
			if (src[i] == '\"')
				double_quotes = double_quotes ^ 1;
			result[number][i - position] = src[i];
		}
		if (i >= len)
			break;
		if (i < len && src[i] == pattern)
			position = i + 1;
		if (i < len && src[i] == pattern && i + 1 < len && result[number][0] != '\0')
		{
			number++;
			result[number] = malloc(MAX_NAME_LINGTH);
			memset(result[number], 0, MAX_NAME_LINGTH);
		}
		i = position;
	}
	if (result[number][0] != '\0')
		number++;
	return number;
}