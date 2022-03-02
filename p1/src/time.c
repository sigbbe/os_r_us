#include "../include/my_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  long int now = unix_timestamp_now();
  char buf[10];
  time_str(now, buf);
  printf("%s\n", buf);
  return 0;
}