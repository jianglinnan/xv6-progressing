#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

#define BUF_SIZE 256

char* fmtname(char *path)
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

void get_files_name(char *path, char **files)
{
	int position = 0;
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;
	
	if((fd = open(path, 0)) < 0)
	{
		printf(2, "ls: cannot open %s\n", path);
		return;
	}
    
	if(fstat(fd, &st) < 0)
	{
		printf(2, "ls: cannot stat %s\n", path);
		close(fd);
		return;
	}
	
	switch(st.type)
	{
		case T_FILE:
			printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
			break;
		case T_DIR:
			if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
			{
				printf(1, "ls: path too long\n");
				break;
			}
			strcpy(buf, path);
			p = buf+strlen(buf);
			*p++ = '/';
			while(read(fd, &de, sizeof(de)) == sizeof(de))
			{
				if(de.inum == 0)
					continue;
				memmove(p, de.name, DIRSIZ);
				p[DIRSIZ] = 0;
				if(stat(buf, &st) < 0)
				{
					printf(1, "ls: cannot stat %s\n", buf);
					continue;
				}
				strcpy(files[position], fmtname(buf));
				position++;
			}
			break;
	}
	close(fd);
}

//复制一个目录
void cpdir(int fd_src, struct stat st, char *src, char *dest)
{
	//初始化文件内容存储
	int i, j;
	char **files = malloc(sizeof(char *) * 32);
	for (i = 0; i < 32; i++)
		files[i] = malloc(sizeof(char) * 128);
	for (i = 0; i < 32; i++)
		for (j = 0; j < 128; j++)
			files[i][j] = 0;
	//获得文件夹中的文件名
	get_files_name(src, files);
	//创建相关文件夹
	mkdir(dest);
	//复制文件
	for (i = 0; i < 32; i++)
	{
		if (files[i][0] == '\0')
			break;
		//获得源文件名
		char src_file_name[128] = {'.', '/'};
		strcpy(src_file_name + 2, src);
		src_file_name[strlen(src) + 2] = '/';
		strcpy(&src_file_name[strlen(src) + 3], files[i]);
		//获得目标文件名
		char dest_file_name[128] = {'.', '/'};
		strcpy(dest_file_name + 2, dest);
		dest_file_name[strlen(dest) + 2] = '/';
		strcpy(&dest_file_name[strlen(dest) + 3], files[i]);
		//打开源文件和目标文件
		int fd_src_inner = open(src_file_name, O_RDONLY);//这里打开出现了问题，返回-1
		int fd_dest_inner = open(dest_file_name, O_WRONLY|O_CREATE);
		//复制文件
		char buf_inner[BUF_SIZE] = {};
		int len = 0;
		printf(1, "%s %d\n", src_file_name, fd_src_inner);
		printf(1, "%s %d\n", dest_file_name, fd_dest_inner);
		while((len = read(fd_src_inner, buf_inner, BUF_SIZE)) > 0)
			write(fd_dest_inner, buf_inner, len);
		close(fd_src_inner);
		close(fd_dest_inner);
	}
	for (i = 0; i < 32; i++)
		free(files[i]);
	free(files);
}

int main(int argc, char *argv[])
{
	//判断参数
	if (argc == 1 || argc == 2)
	{
		printf(1, "please input the command as [cp src_file dest_file]\n");
		exit();
	}
	
	//打开源文件
	int fd_src = open(argv[1], O_RDONLY);
	if (fd_src == -1)
	{
		printf(1, "open source file failed\n");
		exit();
	}
	
	//判断源文件状态是否为文件夹
	struct stat st;
	fstat(fd_src, &st);
	if (st.type == T_DIR)
	{
		cpdir(fd_src, st, argv[1], argv[2]);
		exit();
	}
	
	//判断第二个参数是不是以"/"结尾，如果是，则补全路径
	char path[128] = {};
	strcpy(path, argv[2]);
	int len1 = strlen(argv[1]);
	int len2 = strlen(argv[2]);
	if (argv[2][len2-1] == '/')
	{
		//找到argv[1]中的文件名
		int i = len1 - 1;
		for (; i >= 0; i--)
			if (argv[1][i] == '/')
				break;
		i++;
		strcpy(&path[len2], &argv[1][i]);
	}
	
	//打开目标文件
	int fd_dest = open(path, O_WRONLY|O_CREATE);
	if (fd_dest == -1)
	{
		printf(1, "create dest file failed\n");
		exit();
	}
	
	//复制文件
	char buf[BUF_SIZE] = {};
	int len = 0;
	while((len = read(fd_src, buf, BUF_SIZE)) > 0)
		write(fd_dest, buf, len);
	
	//关闭文件和程序
	close(fd_src);
	close(fd_dest);
	exit();
}
