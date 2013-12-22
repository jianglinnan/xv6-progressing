struct TestStruct
{
	int len;
	char buf[128];
};

#define H_ITEMS 3
#define H_LEN 128
struct HistoryStruct{
	int len;
	int start;
	int current;
	char history[H_ITEMS][H_LEN];
};

#define MAX_EXECMD 64
#define EXECMD_LEN 128

struct ExecutedCmd{
	int len;
	char commands[MAX_EXECMD][EXECMD_LEN];
};
