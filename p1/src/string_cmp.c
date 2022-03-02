#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc == 3) {
    printf("%d\n", strcmp(argv[1], argv[2]));
    return 0;
  } else {
    return 1;
  }
}