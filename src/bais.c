#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  //   char buf[128];
  //   ssize_t n;
  //   char *str = NULL;
  //   size_t len = 0;
  //   while ((n = read(STDIN_FILENO, buf, sizeof buf))) {
  //     if (n < 0) {
  //       if (errno == EAGAIN)
  //         continue;
  //       perror("read");
  //       break;
  //     }
  //     str = realloc(str, len + n + 1);
  //     memcpy(str + len, buf, n);
  //     len += n;
  //     str[len] = '\0';
  //   }
  //   printf("%.*s\n", len, str);
  if (fork() == 0) {
    printf("%s", "CHILD");
  } else {
    printf("%s", "PARENT");
  }
  return 0;
}