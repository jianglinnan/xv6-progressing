#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  // 简单的系统帮助信息
  printf(1, "Thanks for using our xv6 os modification!\n");
  printf(1, "Here are some useful tips for you to start:\n\n");
  printf(1, "Use command \"ls\" to list files in the current directory.\n");
  printf(1, "Use command \"cd DIR\" to change current directory.\n");
  printf(1, "Use command \"rm FILE/DIR\" to remove files or directory.\n");
  printf(1, "Use command \"cp SRC_FILE DST_FILE\" to copy a file.\n");
  printf(1, "Use command \"mv SRC_FILE DST_FILE\" to move a file.\n");
  printf(1, "Use command \"rename SRC_FILE DST_FILE\" to rename a file.\n");
  printf(1, "Use command \"touch FILE1 [FILE2 ...]\" to create a new file.\n");
  printf(1, "Use command \"cat FILE/- [DST_FILE]\" to show or output file contents to file.\n");
  printf(1, "Use command \"script FILE\" to execute a simple script in C grammar.\n");
  printf(1, "Use command \"editor FILE\" to edit a file using simple editor.\n");
  printf(1, "Use command \"make TARGET\" to start a simple GNU Make interpreter.\n");
  printf(1, "Use command \"history\" to see the previous executed commands.\n");
  printf(1, "Use up and down arrow keys to switch to previous executed commands.\n");
  printf(1, "Use Tab to complete command input automatically.\n");

  exit();
}