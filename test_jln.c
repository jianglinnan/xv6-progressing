#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "defs_struct.h"

int
main(int argc, char *argv[])
{
  struct TestStruct ts;
  strcpy(ts.buf, "hello world");
  ts.len = strlen(ts.buf);
  // 测试增加的系统调用
  int result = test(&ts);
  printf(1, "test() return: %d, hongdashen xihuan hehe\n", result);
  //退出程序
  exit();
}

