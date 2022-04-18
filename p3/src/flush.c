#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/flush.h"
#include "../include/linkedlist.h"

static char cwd[255];
static struct LinkedList *list;

/**
 * utility function for writing pointer to char pointers on a single line
 * (separated by white spaces)
 */
void print_char_pointer_pointer(char **args) {
  int count = 0;
  while (1) {
    printf("%s ", args[count]);
    if (args[count] == NULL) {
      break;
    }
    count++;
  }
  printf("\n");
  return;
}

char check_op_order(char *input) {
  int first_op;
  for (int i = 0; i < strlen(input); i++) {
    if (input[i] == '<' || input[i] == '>') {
      first_op = input[i];
      break;
    }
  }
  return (char)first_op;
}

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
CMDArg *parse_args(char *line) {
  CMDArg *args = init_cmd_args();
  set_io_flag(args, line);

  char **io_redir_args = malloc(INPUT_SIZE);
  int count = 0;
  int io_order;
  // io_order = 0 => stdin first
  // io_order = 1 => stdout first

  char *cmd_arg;
  char *newline;

  // no io operations
  if (args->io_flag == -1) {
    char *token;
    while ((token = strtok_r(line, " ", &line))) {
      add_arg(args, token);
    }
  } else {
    char *redir;
    if (args->io_flag == 2) {
      redir = "<>";
    } else if (args->io_flag == 1) {
      redir = ">";
    } else {
      redir = "<";
    }
    // split the command at every whitespace, break when we find the
    // redirection
    while ((cmd_arg = strtok_r(line, " ", &line))) {
      if (strstr(cmd_arg, redir) != NULL) {

        break;
      }
      newline = strchr(cmd_arg, '\n');
      if (newline)
        *newline = 0;
      add_arg(args, cmd_arg);
    }

    // add redirection argument
    char *io_file = strtok(line, " ");
    newline = strchr(io_file, '\n');
    if (newline)
      *newline = 0;

    if (args->io_flag == 1) {
      set_io_out_file(args, io_file);
    } else if (args->io_flag == 0) {
      set_io_in_file(args, io_file);
    }

    // if (args->io_flag == 2) {
    //   redir = "<>";
    //   // head -1 < /etc/passwd > /tmp/foo2
    //   // A < B > C
    //   // A > C < B
    //   // split the command at every whitespace, break when we find the
    //   // redirection
    //   while ((cmd_arg = strtok_r(line, " ", &line))) {
    //     if (cmd_arg[0] == '>' || cmd_arg[0] == '<') {
    //       break;
    //     }
    //     newline = strchr(cmd_arg, '\n');
    //     if (newline)
    //       *newline = 0;
    //     add_arg(args, cmd_arg);
    //   }
    //   //   strtok_r(line, " ", &line);
    //   char *io_file = strtok(line, " ");
    //   printf("io_file: %s\n", io_file);
    //   exit(EXDEV);
    // } else {
    //   if (args->io_flag == 1) {
    //     redir = ">";
    //   } else if (args->io_flag == 0) {
    //     redir = "<";
    //   }
    //   // split the command at every whitespace, break when we find the
    //   // redirection
    //   while ((cmd_arg = strtok_r(line, " ", &line))) {
    //     if (cmd_arg[0] == redir[0])
    //       break;
    //     newline = strchr(cmd_arg, '\n');
    //     if (newline)
    //       *newline = 0;
    //     add_arg(args, cmd_arg);
    //   }

    //   // add redirection argument
    //   char *io_file = strtok(line, " ");
    //   newline = strchr(io_file, '\n');
    //   if (newline)
    //     *newline = 0;

    //   if (args->io_flag == 1) {
    //     set_io_out_file(args, io_file);
    //   } else if (args->io_flag == 0) {
    //     set_io_in_file(args, io_file);
    //   }
    // }
  }
  return args;
}

int no_io_ops(char **args) {
  int status;
  int pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      //   printf("---%ld", strlen(args[0]));
      //   print_args(args);
      printf("execvp(%d) error: %s", errno, strerror(errno));
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
int execute_command(CMDArg *args) {
  pid_t pid;
  int status;

  // first run builtin commands

  // exit
  if (strcmp(args->args[0], "exit") == 0) {
    return 1;
  }

  // display jobs running in the background
  if (strcmp(args->args[0], "jobs") == 0) {
    return 0;
  }

  // change current working directory of the shell
  if (strcmp(args->args[0], "cd") == 0) {
    change_directory(args->args[1]);
    return 0;
  }

  // input and output redirection
  if (args->io_flag == 2) {
    // input and output redirection
    return 0;
  } else if (args->io_flag == 1) {
    // output redirection
    o_ops(args->args, args->io_out_file, 0);
    return 0;
  } else if (args->io_flag == 0) {
    // input redirection
    i_ops(args->args, args->io_in_file);
    return 0;
  } else {
    return no_io_ops(args->args);
  }
}

int o_ops(char **cmd, char *output, int append_flag) {
  int exit_value;
  int flags;
  int stdout = dup(1);
  if (append_flag == 1) {
    flags = (O_RDWR | O_CREAT | O_APPEND);
  } else {
    flags = (O_RDWR | O_CREAT);
  }
  printf("%s", output);
  int file_fd = open(output, flags, 0644);
  if (file_fd == -1) {
    perror("open");
    return errno;
  }
  dup2(file_fd, 1);

  // fork child process
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork");
    return errno;
  } else if (pid == 0) {
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

int i_ops(char **args, char *input) {
  int exit_value;
  int stdin = dup(0);
  int file_fd = open(input, O_RDONLY);
  if (file_fd == -1) {
    perror("open");
    fprintf(stdout, "%d: %s \n", errno, strerror(errno));
    return errno;
  }
  dup2(file_fd, 0);

  // fork child process
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork");
    return 1;
  } else if (pid == 0) {
    execvp(args[0], args);
    perror("execvp");
    exit(1);
  }
  wait(&exit_value);
  dup2(stdin, 0);
  close(stdin);
  close(file_fd);
  return 0;
}

// https://github.com/c-birdsey/mysh

int main(int argc, char *argv[]) {
  int status = 0;
  //   list = makelist();
  //   char *line = NULL;
  //   CMDArg *cmd;
  //   update_cwd();
  //   while (status == 0) {
  //     printf("%s: ", cwd);
  //     line = read_line(stdin);
  //     cmd = parse_args(line);
  //     status = execute_command(cmd);
  //   }
  printf("%s\n", index("head -1 < /etc/passwd > /tmp/foo2", (int)'<'));
  return status != 0;
}