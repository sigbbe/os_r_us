#define _XOPEN_SOURCE
#include <stdio.h>

int main() {
  FILE *fp;
  char line[130];

  fp = popen("netstat -n", "r");

  while (fgets(line, sizeof line, fp)) {
    printf("%s", line);
  }
  pclose(fp);
  return 0;
}