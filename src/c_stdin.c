#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (!isatty(fileno(stdin))) {
    int i = 0;
    char pipe[65536];
    while (-1 != (pipe[i++] = getchar()))
      ;
    fprintf(stdout, "%s\n", pipe);
  }
}
