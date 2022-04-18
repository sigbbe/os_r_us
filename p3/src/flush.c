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

int redir(char **cmd, char *output, int append_flag) {
  int exit_value;
  int flags;
  int stdout = dup(1);
  if (append_flag == 1) {
    flags = (O_RDWR | O_CREAT | O_APPEND);
  } else {
    flags = (O_RDWR | O_CREAT);
  }
  printf("%s\n", output);
  int file_fd = open(output, flags, 0644);
  if (file_fd == -1) {
    perror("open");
    return 0;
  }
  dup2(file_fd, 1);

  // fork child process
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork");
    return -1;
  } else if (pid == 0) {
    printf("[CMD] %s\n", *cmd);
    execvp(cmd[0], cmd);
    perror("execvp");
    exit(1);
  }
  wait(&exit_value);
  dup2(stdout, 1);
  close(stdout);
  close(file_fd);
  return 0;
}

/**
 * Method that takes in a char pointer and executes the program.
 */
int execute_command(char **args) {
  pid_t pid;
  int status;
  int redir_in_flag = 0;
  int redir_out_flag = 0;

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

  int i = 0;
  char **ptr = args;
  for (char *c = *ptr; c; c = *++ptr) {
    if (strchr(c, '<'))
      redir_in_flag = 1;
    if (strchr(c, '>'))
      redir_out_flag = 1;
  }

  // input and output redirection
  if (redir_in_flag && redir_out_flag) {
    // input redirection

  } else if (redir_in_flag) {
    printf("2\n");

    // output redirection
  } else if (redir_out_flag) {
    redir(args, args[3], 0);
    // all other commands
  } else {
    return no_io_ops(args);
  }
  return 0;
}

// https://github.com/c-birdsey/mysh
int get_char_index(char **buf, char chr) {
  int i = 0;
  int j = 0;
  while (buf[i] != NULL) {
    // printf("i: %d, j: %d, strlen(i): %ld\n", i, j, strlen(buf[i]));
    for (int j = 0; j < strlen(buf[i]); j++) {
      if (buf[i][j] == chr) {
        return i;
      }
    }
    i++;
  }
  return -1;
}

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
