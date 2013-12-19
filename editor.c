#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

#define BUF_SIZE 256
#define MAX_LINE_NUMBER 256
#define MAX_LINE_LENGTH 256

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

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		printf(1, "please input the command as [editor file_name]\n");
		exit();
	}
	//存放文件内容
	char *text[MAX_LINE_NUMBER] = {};
	text[0] = malloc(MAX_LINE_LENGTH);
	memset(text[0], 0, MAX_LINE_LENGTH);
	int line_number = 0;
	//尝试打开文件
	int fd = open(argv[1], O_RDWR);
	int file_exists = 0;
	file_exists++;
	//文件存在
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
		printf(1, "\n");
		printf(1, "the contents of the file are:\n");
		int j = 0;
		for (; j <= line_number; j++)
			printf(1, "%d%d%d:%s\n", (j+1)/100, ((j+1)%100)/10, (j+1)%10, text[j]);
		close(fd);
	}
	printf(1, "\n");
	printf(1, "Instructions for use:\n");
	printf(1, "ins-n, insert a line after line n\n");
	printf(1, "mod-n, modify line n\n");
	printf(1, "del-n, delete line n\n");
	printf(1, "ins, insert a line after the last line\n");
	printf(1, "mod, modify the last line\n");
	printf(1, "del, delete the last line\n");
	printf(1, "save, save the file\n");
	printf(1, "exit, exit editor\n");
	printf(1, "\n");
	exit();
}
