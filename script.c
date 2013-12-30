#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

#define BUF_SIZE 256
#define MAX_LINE_NUMBER 256
#define MAX_LINE_LENGTH 256
#define MAX_VARS_NUMBER 16
#define MAX_FUNCTION_NUMBER 16
#define MAX_NAME_LINGTH 16
#define MAX_STACK_LENGTH 16
#define NULL 0

//支持的变量类型
#define VOID 0
#define INT 1
#define CHAR 2
#define STRING 3

//变量类型
typedef struct
{
	int type;
	char name[MAX_NAME_LINGTH];
	char scope[MAX_NAME_LINGTH];
	void *value;
}Var;

//If语句块
typedef struct
{
	int start;
	int end;
	int condition;
}IfBlock;

typedef struct
{
	int if_start;
	int if_end;
	int else_start;
	int else_end;
	int condition;
}IfElseBlock;

//函数类型
typedef struct
{
	//起始行号
	int start;
	//终止行号
	int end;
	//返回类型
	int return_type;
	//函数名
	char name[MAX_NAME_LINGTH];
	//参数类型
	int param_type[MAX_VARS_NUMBER];
	//参数名称
	char param_name[MAX_VARS_NUMBER][MAX_NAME_LINGTH];
}Function;

//栈帧类型
typedef struct
{
	//函数栈帧
	Function function;
	//当前运行位置
	int current;
	//变量列表
	Var vars[MAX_VARS_NUMBER];
	//返回值
	Var return_var;
}Frame;

//全局变量
char *text[MAX_LINE_NUMBER];
Function fun_list[MAX_FUNCTION_NUMBER];
int fun_list_number;
Frame stack_frame[MAX_STACK_LENGTH];
int stack_frame_number;
Var global_vars[MAX_VARS_NUMBER];
int global_vars_number;

//函数声明
void read_script(char *path);
char* strcat_n(char* dest, char* src, int len);
int strcmp_n(const char *str1, const char *str2, int n);
int split(char *src, char pattern, char *result[]);
void free_split(char *array[]);
char* remove_spaces(char *line);
char* remove_semicolon(char *line);
int is_function(char *part);
int is_variable(char *part);
char get_char_value(char *part);
void get_string_value(char *part, char *result);
int find_vars_in_line(char *line, Var *vars, char *scope);
void remove_duplicate_variables(Var *vars);
void find_global_vars();
void find_functions();
int get_fun_finish(int start);
int push_function(char *fun_str);
Var run_function();
int run_line(int line_number);
void get_string_from_var(char *var_name, char *result);
void execute_text();
void execute_line();
void process_text();

int main(int argc, char *argv[])
{
	//判断参数个数
	if (argc < 2)
	{
		//printf(1, "please input the command as [script file]\n");
		exit();
	}
	//读取文件
	read_script(argv[1]);
	//处理
	process_text();
	//退出程序
	exit();
}

//拼接src的前n个字符到dest
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

//比较两个字符串前n个字符的大小
int strcmp_n(const char *str1, const char *str2, int n)
{
	int i = 0;
	for (; i < n; i++)
	{
		if (str1[i] < str2[i])
			return -1;
		if (str1[i] > str2[i])
			return 1;
	}
	return 0;
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

//释放split申请的内存
void free_split(char *array[])
{
	int i = 0;
	for (; array[i] != NULL; i++)
	{
		free(array[i]);
		array[i] = NULL;
	}
}

char* remove_spaces(char *line)
{
	//保存结果
	char result[MAX_LINE_LENGTH] = {};
	int pos = 0;
	//标记是否处于引号中
	int single_quotes = 0;
	int double_quotes = 0;
	//初始化
	int len = strlen(line);
	int i = 0;
	//循环查找
	for (; i < len; i++)
	{
		if (line[i] != '\t')
		{
			//处理引号问题
			if (line[i] != ' ' || single_quotes != 0 || double_quotes != 0)
			{
				result[pos] = line[i];
				pos++;
			}
			//处理声明问题
			else if (line[i] != ' ' ||
				(i >= 4 && strcmp_n(&line[i-4], "void", 4) == 0) ||
				(i >= 3 && strcmp_n(&line[i-3], "int", 3) == 0) ||
				(i >= 4 && strcmp_n(&line[i-4], "char", 4) == 0) ||
				(i >= 6 && strcmp_n(&line[i-6], "string", 6) == 0) ||
				(i >= 6 && strcmp_n(&line[i-6], "return", 6) == 0)
				)
			{
				result[pos] = line[i];
				pos++;
			}
		}
		if (line[i] == '\'')
			single_quotes = single_quotes ^ 1;
		if (line[i] == '\"')
			double_quotes = double_quotes ^ 1;
	}
	memset(line, 0, MAX_LINE_LENGTH);
	strcpy(line, result);
	return line;
}

//去掉分号和之后的东西，因此一行中有多个语句将只识别第一个
//可以认为分号之后的内容为注释
char* remove_semicolon(char *line)
{
	//标记是否处于引号中
	int single_quotes = 0;
	int double_quotes = 0;
	
	int i = 0;
	int len = strlen(line);
	for (; i < len && (line[i] != ';' || single_quotes || double_quotes); i++)
	{
		if (line[i] == '\'')
			single_quotes = single_quotes ^ 1;
		if (line[i] == '\"')
			double_quotes = double_quotes ^ 1;
	}
	for (; i < len; i++)
		line[i] = '\0';
	return line;
}

//读文件
void read_script(char *path)
{
	//打开文件
	int fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		//printf(1, "can't open the file\n");
		exit();
	}
	//读取内容
	int line_number = 0;
	text[0] = malloc(MAX_LINE_LENGTH);
	memset(text[0], 0, MAX_LINE_LENGTH);
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

int is_function(char *part)
{
	if (part[0] == '\0' || part[0] == '(' || part[0] == ')')
		return 0;
	int len = strlen(part);
	int i = 0;
	int ok = 0;
	for (i = 1; i < len; i++)
	{
		if (part[i] == '(')
		{
			ok++;
			break;
		}
	}
	for (i = i + 1; i < len; i++)
		if (part[i] == ')' && ok == 1)
			return 1;
	return 0;
}

int is_variable(char *part)
{
	int i = 0;
	if (part[0] == '\0')
		return 0;
	for (; part[i] != '\0'; i++)
		if (part[i] == '(' || part[i] == ')')
			return 0;
	return 1;
}

char get_char_value(char *part)
{
	int len = strlen(part);
	if (len <= 2)
		return '\0';
	else if (len == 3)
		return part[1];
	else if (len == 4 && part[1] == '\\')
	{
		if (part[2] == 'a')
			return '\a';
		if (part[2] == 'b')
			return '\b';
		if (part[2] == 'f')
			return '\f';
		if (part[2] == 'n')
			return '\n';
		if (part[2] == 'r')
			return '\r';
		if (part[2] == 't')
			return '\t';
		if (part[2] == 'v')
			return '\v';
		if (part[2] == '\\')
			return '\\';
		if (part[2] == '\'')
			return '\'';
		if (part[2] == '\"')
			return '\"';
		if (part[2] == '\0')
			return '\0';
	}
	return '\0';
}

void get_string_value(char *part, char *result)
{
	int pos = 0;
	int len = strlen(part);
	if (len < 2)
		return;
	int i = 1;
	for (; i < len - 1; i++)
	{
		if (part[i] != '\\')
			result[pos] = part[i];
		else if (i < len - 2)
		{
			char one_char[5] = {};
			one_char[0] = '\'';
			one_char[1] = '\\';
			one_char[2] = part[i+1];
			one_char[3] = '\'';
			result[pos] = get_char_value(one_char);
			i++;
		}
		pos++;
	}
}

//获得一行的变量，返回获得的个数
int find_vars_in_line(char *line, Var *vars, char *scope)
{
	int count = 0;
	char *split_vars[MAX_NAME_LINGTH] = {};
	if (strcmp_n(line, "int ", 4) == 0 && is_variable(line+4))
	{
		int n = split(line+4, ',', split_vars);
		int j = 0;
		for (; j < n; j++)
		{
			//判断是int a,b,c还是int a=1,b=2,c=3
			char *split_expression[MAX_NAME_LINGTH] = {};
			int m = split(split_vars[j], '=', split_expression);
			//没有赋值的变量
			if (m == 1)
			{
				vars[count].type = INT;
				strcpy(vars[count].name, split_expression[0]);
				strcpy(vars[count].scope, scope);
				vars[count].value = malloc(sizeof(int));
				*(int *)(vars[count].value) = 0;
				count++;
			}
			if (m >= 2)
			{
				vars[count].type = INT;
				strcpy(vars[count].name, split_expression[0]);
				strcpy(vars[count].scope, scope);
				vars[count].value = malloc(sizeof(int));
				*(int *)(vars[count].value) = atoi(split_expression[1]);
				count++;
			}
			free_split(split_expression);
		}
		free_split(split_vars);
	}
	if (strcmp_n(line, "char ", 5) == 0 && is_variable(line+5))
	{
		int n = split(line+5, ',', split_vars);
		int j = 0;
		for (; j < n; j++)
		{
			//判断是char a,b,c还是char a='a',b='b',c='c'
			char *split_expression[MAX_NAME_LINGTH] = {};
			int m = split(split_vars[j], '=', split_expression);
			//没有赋值的变量
			if (m == 1)
			{
				vars[count].type = CHAR;
				strcpy(vars[count].name, split_expression[0]);
				strcpy(vars[count].scope, scope);
				vars[count].value = malloc(sizeof(char));
				*(char *)(vars[count].value) = '\0';
				count++;
			}
			if (m >= 2)
			{
				vars[count].type = CHAR;
				strcpy(vars[count].name, split_expression[0]);
				strcpy(vars[count].scope, scope);
				vars[count].value = malloc(sizeof(char));
				*(char *)(vars[count].value) = get_char_value(split_expression[1]);
				count++;
			}
			free_split(split_expression);
		}
		free_split(split_vars);
	}
	if (strcmp_n(line, "string ", 7) == 0 && is_variable(line+7))
	{
		int n = split(line+7, ',', split_vars);
		int j = 0;
		for (; j < n; j++)
		{
			//判断是string a,b,c还是string a="a",b="b",c="c"
			char *split_expression[MAX_NAME_LINGTH] = {};
			int m = split(split_vars[j], '=', split_expression);
			//没有赋值的变量
			if (m == 1)
			{
				vars[count].type = STRING;
				strcpy(vars[count].name, split_expression[0]);
				strcpy(vars[count].scope, scope);
				vars[count].value = malloc(MAX_LINE_LENGTH);
				memset(vars[count].value, 0, MAX_LINE_LENGTH);
				count++;
			}
			if (m >= 2)
			{
				vars[count].type = STRING;
				strcpy(vars[count].name, split_expression[0]);
				strcpy(vars[count].scope, scope);
				vars[count].value = malloc(MAX_LINE_LENGTH);
				memset(vars[count].value, 0, MAX_LINE_LENGTH);
				get_string_value(split_expression[1], vars[count].value);
				count++;
			}
			free_split(split_expression);
		}
		free_split(split_vars);
	}
	return count;
}

//删除重复变量
void remove_duplicate_variables(Var *vars)
{
	int count = 0;
	for (; count < MAX_VARS_NUMBER; count++)
		if (vars[count].name[0] == '\0')
			break;
	int i = 0, j = 0;
	int invalid[MAX_VARS_NUMBER] = {};
	for (i = 0; i < count; i++)
		for (j = 0; j < i; j++)
			if (strcmp(vars[i].name, vars[j].name) == 0)
				invalid[j] = 1;
	Var tmp[MAX_VARS_NUMBER];
	memset(tmp, 0, sizeof(Var) * MAX_VARS_NUMBER);
	int pos = 0;
	for (i = 0; i < count; i++)
	{
		if (invalid[i] == 0)
		{
			tmp[pos].type = vars[i].type;
			strcpy(tmp[pos].name, vars[i].name);
			strcpy(tmp[pos].scope, vars[i].scope);
			tmp[pos].value = vars[i].value;
			pos++;
		}
	}
	memset(vars, 0, sizeof(Var) * MAX_VARS_NUMBER);
	for (i = 0; i < pos; i++)
	{
		vars[i].type = tmp[i].type;
		strcpy(vars[i].name, tmp[i].name);
		strcpy(vars[i].scope, tmp[i].scope);
		vars[i].value = tmp[i].value;
	}
}

void find_global_vars()
{
	int i = 0, j = 0;
	char scope[1] = {};
	//判断那些行不属于函数部分
	int in_fun[MAX_LINE_NUMBER] = {};
	for (i = 0; i < fun_list_number; i++)
		for (j = fun_list[i].start; j <= fun_list[i].end; j++)
			in_fun[j] = 1;
	//加入全局变量
	for (i = 0; text[i] != 0; i++)
	{
		if (in_fun[i] == 1)
			continue;
		global_vars_number += find_vars_in_line(text[i], global_vars + global_vars_number, scope);
	}
	//去重
	remove_duplicate_variables(global_vars);
}

void find_functions()
{
	int i = 0;
	for (; text[i] != NULL; i++)
	{
		int pos1 = -1;
		int pos2 = -1;
		if (strcmp_n(text[i], "int ", 4) == 0 && is_function(text[i]+4))
		{
			int j = 4;
			for (; text[i][j] != '('; j++)
				;
			pos1 = j;
			for (j = pos1+1; text[i][j] != ')'; j++)
				;
			pos2 = j;
			//设定函数名字
			strcat_n(fun_list[fun_list_number].name, text[i]+4, pos1-4);
			//设定函数返回类型
			fun_list[fun_list_number].return_type = INT;
		}
		//函数类型
		if (strcmp_n(text[i], "char ", 5) == 0 && is_function(text[i]+5))
		{
			int j = 5;
			for (; text[i][j] != '('; j++)
				;
			pos1 = j;
			for (j = pos1+1; text[i][j] != ')'; j++)
				;
			pos2 = j;
			//设定函数名字
			strcat_n(fun_list[fun_list_number].name, text[i]+5, pos1-5);
			//设定函数返回类型
			fun_list[fun_list_number].return_type = CHAR;
		}
		if (strcmp_n(text[i], "string ", 7) == 0 && is_function(text[i]+7))
		{
			int j = 7;
			for (; text[i][j] != '('; j++)
				;
			pos1 = j;
			for (j = pos1+1; text[i][j] != ')'; j++)
				;
			pos2 = j;
			//设定函数名字
			strcat_n(fun_list[fun_list_number].name, text[i]+7, pos1-7);
			//设定函数返回类型
			fun_list[fun_list_number].return_type = STRING;
		}
		if (strcmp_n(text[i], "void ", 5) == 0 && is_function(text[i]+5))
		{
			int j = 5;
			for (; text[i][j] != '('; j++)
				;
			pos1 = j;
			for (j = pos1+1; text[i][j] != ')'; j++)
				;
			pos2 = j;
			//设定函数名字
			strcat_n(fun_list[fun_list_number].name, text[i]+5, pos1-5);
			//设定函数返回类型
			fun_list[fun_list_number].return_type = VOID;
		}
		if (pos1 != -1 && pos2 != -1)
		{
			int j = 0;
			//寻找参数类型
			char to_split[MAX_LINE_NUMBER] = {};
			strcat_n(to_split, text[i]+pos1+1, pos2-pos1-1);
			char *split_result[MAX_LINE_NUMBER] = {};
			int number = split(to_split, ',', split_result);
			Var params[MAX_VARS_NUMBER];
			int params_number = 0;
			memset(params, 0, sizeof(Var) * MAX_VARS_NUMBER);
			for (j = 0; j < number; j++)
				params_number += find_vars_in_line(split_result[j], params+params_number, fun_list[fun_list_number].name);
			for (j = 0; j < params_number; j++)
			{
				fun_list[fun_list_number].param_type[j] = params[j].type;
				strcpy(fun_list[fun_list_number].param_name[j], params[j].name);
			}
			free_split(split_result);
			//设定函数起点
			fun_list[fun_list_number].start = i;
			//寻找函数终点
			fun_list[fun_list_number].end = get_fun_finish(i);
			//增加函数计数
			fun_list_number++;
		}
	}
}

int get_fun_finish(int start)
{
	int count = 0;
	int flag = 0;
	int i = start + 1;
	for (; i < MAX_LINE_NUMBER && text[i] != NULL && (flag == 0 || count > 0); i++)
	{
		if (text[i][0] == '{')
		{
			count++;
			flag = 1;
		}
		if (text[i][0] == '}')
		{
			count--;
			flag = 1;
		}
	}
	i--;
	return i;
}

int push_function(char *fun_str)
{
	//找到左括号和右括号的位置
	int i = 0;
	for(; fun_str[i] != '(' && i < MAX_LINE_LENGTH; i++)
		;
	int pos1 = i;
	if (pos1 < 1)
	{
		//printf(1, "error, it's not function\n");
		//exit();
	}
	for(; fun_str[i] != ')' && i < MAX_LINE_LENGTH; i++)
		;
	int pos2 = i;
	//寻找函数
	char fun_name[MAX_NAME_LINGTH] = {};
	strcat_n(fun_name, fun_str, pos1);
	for (i = 0; i < fun_list_number; i++)
		if (strcmp(fun_name, fun_list[i].name) == 0)
			break;
	if (i == fun_list_number)
	{
		//printf(1, "error, can't find function %s\n", fun_name);
		//exit();
	}
	stack_frame[stack_frame_number].function = fun_list[i];
	stack_frame[stack_frame_number].current = fun_list[i].start + 2;
	//处理参数
	char param_name[MAX_LINE_LENGTH] = {};
	strcat_n(param_name, fun_str+pos1+1, pos2-pos1-1);
	char *split_param[MAX_VARS_NUMBER] = {};
	int param_number = split(param_name, ',', split_param);
	for (i = 0; i < param_number; i++)
	{
		int len = strlen(split_param[i]);
		//整数
		if (atoi(split_param[i]) != 0 || split_param[i][0] == '0')
		{
			if (stack_frame[stack_frame_number].function.param_type[i] != INT)
			{
				//printf(1, "parameter type error\n");
				//exit();
			}
			stack_frame[stack_frame_number].vars[i].type = INT;
			strcpy(stack_frame[stack_frame_number].vars[i].name, stack_frame[stack_frame_number].function.param_name[i]);
			strcpy(stack_frame[stack_frame_number].vars[i].scope, stack_frame[stack_frame_number].function.name);
			stack_frame[stack_frame_number].vars[i].value = malloc(sizeof(int));
			*(int *)stack_frame[stack_frame_number].vars[i].value = atoi(split_param[i]);
		}
		//字符
		else if (split_param[i][0] == '\'' && split_param[i][len-1] == '\'')
		{
			if (stack_frame[stack_frame_number].function.param_type[i] != CHAR)
			{
				//printf(1, "parameter type error\n");
				//exit();
			}
			stack_frame[stack_frame_number].vars[i].type = CHAR;
			strcpy(stack_frame[stack_frame_number].vars[i].name, stack_frame[stack_frame_number].function.param_name[i]);
			strcpy(stack_frame[stack_frame_number].vars[i].scope, stack_frame[stack_frame_number].function.name);
			stack_frame[stack_frame_number].vars[i].value = malloc(sizeof(char));
			*(char *)stack_frame[stack_frame_number].vars[i].value = get_char_value(split_param[i]);
		}
		//字符串
		else if (split_param[i][0] == '\"' && split_param[i][len-1] == '\"')
		{
			if (stack_frame[stack_frame_number].function.param_type[i] != STRING)
			{
				//printf(1, "parameter type error\n");
				//exit();
			}
			stack_frame[stack_frame_number].vars[i].type = STRING;
			strcpy(stack_frame[stack_frame_number].vars[i].name, stack_frame[stack_frame_number].function.param_name[i]);
			strcpy(stack_frame[stack_frame_number].vars[i].scope, stack_frame[stack_frame_number].function.name);
			stack_frame[stack_frame_number].vars[i].value = malloc(sizeof(char) * MAX_LINE_LENGTH);
			get_string_value(split_param[i], stack_frame[stack_frame_number].vars[i].value);
		}
		//变量
		else
		{
			//未实现
			printf(1, "transfer variable not implement\n");
		}
	}
	stack_frame_number++;
	return stack_frame[stack_frame_number - 1].current;
}

void execute_text()
{
	//初始化操作
	push_function("main()");
	run_function();
}

Var run_function()
{
	int start = stack_frame[stack_frame_number - 1].current;
	int end = stack_frame[stack_frame_number - 1].function.end - 1;
	int i = start;
	for (; i <= end; i++)
	{
		//run_line()一次处理了n行
		int n = run_line(i);
		i += (n-1);
	}
	Var return_val;
	return return_val;
}

int run_line(int line_number)
{
	printf(1, "%s\n", text[line_number]);
	//系统命令
	if (strcmp_n(text[line_number], "system(", 7) == 0)
	{
		char com[MAX_LINE_LENGTH] = {};
		char str[MAX_LINE_LENGTH] = {};
		strcat_n(com, text[line_number] + 7, strlen(text[line_number])-8);
		//双引号命令
		if (com[0] == '\"')
			get_string_value(com, str);
		else
			get_string_from_var(com, str);
	}
	//定义性语句
	int local_vars_number = 0;
	for (; stack_frame[stack_frame_number - 1].vars[local_vars_number].type != 0; local_vars_number++)
		;
	int number = find_vars_in_line(text[line_number], stack_frame[stack_frame_number - 1].vars + local_vars_number, stack_frame[stack_frame_number].function.name);
	local_vars_number += number;
	remove_duplicate_variables(stack_frame[stack_frame_number - 1].vars);
	if (number > 0)
		return 1;
	//if逻辑块和if-else逻辑块
	//for逻辑块
	//不接收返回值的函数调用逻辑块
	if(is_function(text[line_number]))
	{
		push_function(text[line_number]);
		run_function();
		return 1;
	}
	//赋值语句处理
	return 1;
}

void get_string_from_var(char *var_name, char *result)
{
	int i = 0;
	int exists = 0;
	//局部变量中是否存在
	for (; stack_frame[stack_frame_number - 1].vars[i].type != VOID; i++)
	{
		Var local = stack_frame[stack_frame_number].vars[i];
		if (local.type == STRING && strcmp(local.name, var_name) == 0)
		{
			strcpy(result, (char *)local.value);
			exists = 1;
			break;
		}
	}
	//全局变量中是否存在
	if (exists == 0)
	{
		for (i = 0; i < global_vars_number; i++)
		{
			if (global_vars[i].type == STRING && strcmp(global_vars[i].name, var_name) == 0)
			{
				strcpy(result, (char *)global_vars[i].value);
				exists = 1;
				break;
			}
		}
	}
	//不存在该变量
	if (exists == 0)
	{
		//printf(1, "can't find variable %s", com);
		//exit();
	}
	return;
}

void process_text()
{
	//删除无关的空格和tab
	int i = 0;
	for (; text[i] != NULL; i++)
	{
		remove_spaces(text[i]);
		remove_semicolon(text[i]);
	}
	
	//找到所有的函数
	find_functions();
	//找到所有的全局变量
	find_global_vars();
	//执行
	execute_text();
}
