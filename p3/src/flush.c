#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define TOKEN_BUFSIZE 64
#define INPUT_SIZE 4096
#define DELIM_CHARS " \t\r\n\a"

/* PROTOTYPES */
void pipe_parse(char *input);
void io_parse(char *input);
void pipe_handler(char **pipe_args, int pipe_count);
void io_redir(char **cmd, char *input, char *output, int append_flag);

void input_redir(char **cmd, char *input);
void output_redir(char **cmd, char *output, int append_flag);
void print_args(char **args);

static char cwd[255];

/**
 * Struct for agruements. Holds the char ** pointer to the list of arguments.
 * Also keeps track of io_flag, append_flag, number of arguments.
 */
typedef struct CmdArgs {
  char **args;
  char *io_in_file;
  char *io_out_file;
  int io_flag;
  // io_flag = -1 => no redirect;
  // io_flag = 0 => input redirect;
  // io_flag = 1 => output redirect;
  // io_flag = 2 => input/output redirect;
  int append_flag;
  int arg_count;
} cmd_args;

/**
 * Function for initializing cmd_args struct
 */
cmd_args *init_cmd_args() {
  cmd_args *args = malloc(sizeof(cmd_args));
  args->args = malloc(sizeof(char *) * TOKEN_BUFSIZE);
  args->io_in_file = NULL;
  args->io_out_file = NULL;
  args->io_flag = -1;
  args->append_flag = 0;
  args->arg_count = 0;
  return args;
}

/**
 * Function for descructing cmd_args struct
 */
void destroy_cmd_args(cmd_args *args) {
  free(args->args);
  free(args);
}

/**
 *
 * Function for adding another argument to the cmd_args.args array.
 * If the array is full realloc the array and add the new element to the end.
 *
 */
void add_arg(cmd_args *args, char *arg) {
  if (args->arg_count >= TOKEN_BUFSIZE) {
    args->args = realloc(args->args, sizeof(char *) * (TOKEN_BUFSIZE * 2));
  }
  args->args[args->arg_count++] = arg;
}

/**
 * Function for setting the io_flag. receives char pointer as argument, and
 looks for '<' and '>' characters.
 * io_flag = -1 => no redirect;
 * io_flag = 0 => input redirect;
 * io_flag = 1 => output redirect;
 * io_flag = 2 => input/output redirect;
 */
void set_io_flag(cmd_args *args, char *arg) {
  if ((strchr(arg, '<')) && (strchr(arg, '>'))) {
    (*args).io_flag = 2;
  } else if (strchr(arg, '<')) {
    (*args).io_flag = 0;
  } else if (strchr(arg, '>')) {
    (*args).io_flag = 1;
  } else {
    (*args).io_flag = -1;
  }
}

/**
 * Setter function for io_file property of cmd_args struct
 */
void set_io_in_file(cmd_args *args, char *file) { args->io_in_file = file; }

/**
 * Setter function for io_file2 property of cmd_args struct
 */
void set_io_out_file(cmd_args *args, char *file) { args->io_out_file = file; }

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
cmd_args *parse_args(char *line) {
  cmd_args *args = init_cmd_args();
  set_io_flag(args, line);

  char **io_redir_args = malloc(INPUT_SIZE);
  int count = 0;

  printf("%d\n", args->io_flag);

  // no io operations
  if (args->io_flag == -1) {
    char *token;
    while ((token = strtok_r(line, " ", &line))) {
      add_arg(args, token);
    }
  } else {
    char *redir;
    if (args->io_flag == 2) {
      // redir = "<>";



    } else {
      if (args->io_flag == 1) {
        redir = ">";
      } else if (args->io_flag == 0) {
        redir = "<";
      }
      char *cmd_arg;
      while ((cmd_arg = strtok_r(line, " ", &line))) {
        if (cmd_arg[0] == redir[0])
          break;
        add_arg(args, cmd_arg);
      }
      // strchr(line, redir[0]);
      if (args->io_flag == 1) {
        set_io_out_file(args, strtok(line, " "));
      } else if (args->io_flag == 0) {
        set_io_in_file(args, strtok(line, " "));
      }
    }

  }
  return args;
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
int execute_command(cmd_args *args) {
  pid_t pid;
  int status;

  // first run builtin commands

  // exit
  if (strcmp(args->args[0], "exit") == 0) {
    return 1;
  }

  // change current working directory of the shell
  if (strcmp(args->args[0], "jobs") == 0) {
    print_jobs();
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
  } else if (args->io_flag == 1) {
    // output redirection
    output_redir(args->args, args->io_out_file, 0);
  } else if (args->io_flag == 0) {
    // input redirection
    printf("INFILE: %s\n", args->io_in_file);
    input_redir(args->args, args->io_in_file);
  } else {
    return no_io_ops(args->args);
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

void output_redir(char **cmd, char *output, int append_flag) {
  int exit_value;
  int flags;
  int stdout = dup(1);
  if (append_flag == 1) {
    flags = (O_RDWR | O_CREAT | O_APPEND);
  } else {
    flags = (O_RDWR | O_CREAT);
  }
  int file_fd = open(output, flags, 0644);
  if (file_fd == -1) {
    perror("open");
    return;
  }
  dup2(file_fd, 1);

  // fork child process
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork");
    return;
  } else if (pid == 0) {
    execvp(cmd[0], cmd);
    perror("execvp");
    exit(1);
  }
  wait(&exit_value);
  dup2(stdout, 1);
  close(stdout);
  close(file_fd);
  return;
}

void input_redir(char **cmd, char *input) {
  int exit_value;
  int stdin = dup(0);
  int file_fd = open(input, O_RDONLY);
  if (file_fd == -1) {
    perror("open");
    fprintf(stdout, "%d: %s \n", errno,  strerror(errno));
    return;
  }
  dup2(file_fd, 0);

  // fork child process
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork");
    return;
  } else if (pid == 0) {
    execvp(cmd[0], cmd);
    perror("execvp");
    exit(1);
  }
  wait(&exit_value);
  dup2(stdin, 0);
  close(stdin);
  close(file_fd);
  return;
}

/**
 *
 */
int main(int argc, char *argv[]) {
  int status = 0;
  char *line = NULL;
  cmd_args *cmd;
  update_cwd();
  while (status == 0) {
    printf("%s: ", cwd);
    line = read_line(stdin);
    cmd = parse_args(line);
    print_args(cmd->args);
    status = execute_command(cmd);
  }
  return status != 0;
}

void print_args(char **args) {
  int count = 0;
  while (1) {
    printf("Arg %d: %s\n", count, args[count]);
    if (args[count] == NULL) {
      break;
    }
    count++;
  }
  printf("\n");
  return;
}