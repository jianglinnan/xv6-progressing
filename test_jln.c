#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  // 测试增加的系统调用
  int result = test();
  printf(1, "test() return: %d, hongdashen xihuan hehe\n", result);
  //退出程序
  exit();
}

