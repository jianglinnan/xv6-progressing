#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "defs_struct.h"
#define MAX_NAME_LINGTH 16

int split(char *src, char pattern, char *result[]);
char* combinetwostring(char* src, int st, int ed, char* dest);

int
main(int argc, char *argv[])
{
	/*
	char *testres[256] = {};
	split("mkdir    test1.o    test2.o    test3.o", ' ', testres);
	printf(1, "%s\n", testres[0]);
	if (fork() == 0)
	{
		exec(testres[0], testres);
	}
	wait();
	*/
	char *result = combinetwostring("abcdefghijklmn", 13, 13, "111111");
	printf(1, "%s\n", result);
  	exit();
  	
}

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
