#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  execl("echo", "/bin/echo", "Hello World", NULL);
  printf("%d\n", argc);
  for (int i = 0; i < argc - 1; i++) {
    printf("%s\n", argv[i]);
  }
  return 0;
}