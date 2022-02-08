#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void run_cmd(char *cmd, char *const envp[__restrict_arr]) {
  pid_t pid;
  char *argv[] = {"sh", "-c", cmd, NULL};
  int status;
  sleep(10);
  printf("Run: %s\n", cmd);
  status = posix_spawn(&pid, "/bin/sh", NULL, NULL, argv, envp);
  if (status == 0) {
    printf("Child pid: %d\n", pid);
    if (waitpid(pid, &status, 0) != -1) {
      printf("Child exited with status: %i\n", status);
    } else {
      perror("waitpid");
    }
  } else {
    printf("posix_spawn: %s\n", strerror(status));
  }
}

int main(int arc, char **argv, char **envp) {
  //   printf("%s\n", *envp);
  //   printf("CURRENT_PID=%d\n", getpid());
  run_cmd("google-chrome", envp);
  return 0;
}
