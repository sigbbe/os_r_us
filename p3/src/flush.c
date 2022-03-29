#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define TOKEN_BUFSIZE 64
#define INPUT_SIZE 4096
#define DELIM_CHARS " \t\r\n\a"

/**
 * Get the current working directory as a char pointer.
 * put that in a buffer. Then append a colon :)
 *
 * @param buffer the buffer to put the cwd in
 *
 */

void get_cwd(char *buffer) {
  char *cwd = getcwd(buffer, 1024);
  if (cwd == NULL) {
    perror("getcwd() error");
    exit(EXIT_FAILURE);
  }
  strcat(buffer, ":");
}

/**
 * Get the next token from the input stream.
 *
 * @param input the input stream
 */
char *read_line(FILE *stream) {
  char *line = NULL;
  size_t bufsize = 0;
  if (getline(&line, &bufsize, stream) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);
    } else {
      perror("read_line");
      exit(EXIT_FAILURE);
    }
  }
  return line;
}

/**
 * Method that parses the name of a program and n number of arguments
 * received in the form of a char pointer.
 */
char **parse_args(char *line) {
  int bufsize = TOKEN_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  '
  '

      if (!tokens) {
    printf("Error");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, DELIM_CHARS);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += TOKEN_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));

      if (!tokens) {
        printf("Error");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, DELIM_CHARS);
  }

  tokens[position] = NULL;
  return tokens;
}
// void parse_args(char inp[], char *args[], int *n) {
//   int i = 0;
//   char *token = strtok(inp, DELIM_CHARS);
//   while (token != NULL) {
//     args[i] = token;
//     i++;
//     token = strtok(NULL, DELIM_CHARS);
//   }
//   *n = i;
// }

/**
 * Method that takes in a char pointer and changes working directory for the
 * current process.
 */
void change_directory(const char *restrict pathname) {
  if (pathname == NULL || strcmp(pathname, "") == 0) {
    return;
  }
  if (chdir(pathname) == -1) {
    perror("chdir() error");
    exit(EXIT_FAILURE);
  }
}

int no_io_ops(char **args) {
  int status;
  int pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("execvp() error");
      exit(EXIT_FAILURE);
    }
  } else if (pid < 0) {
    perror("fork() error");
    exit(EXIT_FAILURE);
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 0;
}

/**
 * Method that takes in a char pointer and executes the program.
 */
int execute_command(char **args) {
  pid_t pid;
  int status;
  if (strcmp(args[0], "exit") == 0) {
    return 1;
  }
  printf("%s\n", args[1]);
  if ((strchr(*args, '<')) || (strstr(*args, ">"))) {
    printf("Invalid command %s\n", *args);
  } else {
    return no_io_ops(args);
  }
  return 0;
}

/**
 * Parse the command line arguments.
 * argv[0] is the name of this program.
 * argv[1] is the name of the program that we will invoke.
 * All other argv elements are passed to the invoked program.
 *
 * @param int argc number of arguments
 * @param char **argv[] array of arguments
 *
 */

int main(int argc, char *argv[]) {
  int status = 0;
  int n_args;
  char *cwd = malloc(1024);
  char *line = NULL;
  char **cmd;
  while (status == 0) {
    get_cwd(cwd);
    printf("%s ", cwd);
    line = read_line(stdin);
    cmd = parse_args(line);
    printf("---%s\n", cmd[2]);
    status = execute_command(cmd);
  }
  return 0;
}
