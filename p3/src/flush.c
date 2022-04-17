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

/* PROTOTYPES */
void pipe_parse(char *input);
void io_parse(char *input);
void input_redir(char **cmd, char *input);
void output_redir(char **cmd, char *output, int append_flag);
void pipe_handler(char **pipe_args, int pipe_count);
void io_redir(char **cmd, char *input, char *output, int append_flag);

static char cwd[255];

/**
 * update the static char array cwd with the current working directory
 */
void update_cwd() {
  char *_cwd = getcwd(cwd, 255);
  if (_cwd == NULL) {
    perror("getcwd() error");
    exit(EXIT_FAILURE);
  }
}

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
  update_cwd();
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

  // first run builtin commands

  // exit
  if (strcmp(args[0], "exit") == 0) {
    return 1;
  }

  // change current working directory of the shell
  if (strcmp(args[0], "cd") == 0) {
    change_directory(args[1]);
    return 0;
  }

  // input redirection
  if (strchr(*args, '<')) {
    printf("input redirection\n");
  } else if (strstr(*args, ">")) /* output redirection */ {
    printf("output redirection\n");
  } else /* all other commands */ {
    return no_io_ops(args);
  }
  return 0;
}

// https://github.com/c-birdsey/mysh

/**
 *
 */
int main(int argc, char *argv[]) {
  int status = 0;
  char *line = NULL;
  char **cmd;
  update_cwd();
  while (status == 0) {
    printf("%s: ", cwd);
    line = read_line(stdin);
    cmd = parse_args(line);
    status = execute_command(cmd);
  }
  return status != 0;
}
