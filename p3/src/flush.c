#include <asm-generic/errno-base.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/flush.h"
#include "../include/linkedlist.h"

char cwd[255];
struct LinkedList *jobs;
pid_t parent_pid;

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

int index_of(char *input, char c) {
  char *e;
  int index;
  e = strchr(input, c);
  index = (int)(e - input);
  if (index < 0 || index > strlen(input)) {
    return -1;
  } else {
    return index;
  }
}

int is_white_space(char input) {
  return (input == DELIM_CHARS[0] || input == DELIM_CHARS[1] ||
          input == DELIM_CHARS[2] || input == DELIM_CHARS[3] ||
          input == DELIM_CHARS[4]);
}

int all(char **ptr, int (*fn_ptr)(char *)) {
  int i = 0;
  while (ptr[i] != NULL) {
    if (fn_ptr(ptr[i]) == 0) {
      return 0;
    }
    i++;
  }
  return 1;
}

/**
 * Removes all the white spaces (defined in DELIM_CHARS) from the input string
 */
void remove_all_whitespace_chars(char str[], char *ptr) {
  int i = 0, j = 0, len;
  len = strlen(ptr); // len stores the length of the input string

  // till string doesn't terminate
  while (ptr[i] != '\0') {
    // if the char is not a white space
    if (!is_white_space(ptr[i])) {
      // incrementing index j only when
      // the char is not space
      str[j++] = ptr[i];
    }
    // i is the index of the actual string and
    // is incremented irrespective of the spaces
    i++;
  }
  str[j] = '\0';
}

/**
 * check if the given process has exited
 */
int process_has_exited(pid_t pid) { return (getpgid(pid) < 0); }

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

  char *cmd_arg;

  // no io operations
  if (args->io_flag == -1) {
    char *token;
    while ((token = strtok_r(line, " ", &line))) {
      add_arg(args, token);
    }
  } else {
    char *redir;
    int io_order;
    // io_order = 0 => stdin first
    // io_order = 1 => stdout first
    if (args->io_flag == 2) {
      redir = "<>";
    } else if (args->io_flag == 1) {
      redir = ">>";
      io_order = 1;
    } else {
      redir = "<<";
      io_order = 0;
    }
    // split the command at every whitespace, break when we find the
    // redirection
    while ((cmd_arg = strtok_r(line, " ", &line))) {
      if (strchr(cmd_arg, redir[0]) != NULL ||
          strchr(cmd_arg, redir[1]) != NULL) {
        if (args->io_flag == 2) {
          io_order = index_of(cmd_arg, '>') > index_of(cmd_arg, '<');
        }
        break;
      }
      remove_all_whitespace_chars(cmd_arg, cmd_arg);
      add_arg(args, cmd_arg);
    }

    char io_file[100];
    remove_all_whitespace_chars(io_file, strtok(line, " "));

    if (args->io_flag == 1) {
      set_io_out_file(args, io_file);
    } else if (args->io_flag == 0) {
      set_io_in_file(args, io_file);
    } else {
      char io_file_2[100];
      remove_all_whitespace_chars(io_file_2, strtok(NULL, &redir[!io_order]));
      if (io_order) {
        set_io_in_file(args, io_file_2);
        set_io_out_file(args, io_file);
      } else {
        set_io_in_file(args, io_file);
        set_io_out_file(args, io_file_2);
      }
    }
  }
  return args;
}

int no_io_ops(char **args) {
  int status;
  int pid = fork();
  if (pid == 0) {
    execvp(args[0], args);
    fprintf(stderr, "execvp(%d) error: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    fprintf(stderr, "fork(%d) error: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return EXIT_SUCCESS;
}

int is_background_command(char **command) {
  int i = 0;
  int j = 1;
  int len;
  if (command == NULL || command[0] == NULL) {
    return 0;
  }
  while (command[i + 1] != NULL) {
    if (strlen(command[i + 1]) == 0) {
      break;
    }
    i++;
  }
  len = strlen(command[i]);
  while (len - j >= 0) {
    char letter = command[i][len - j];
    if (letter == '&') {
      return 1;
    } else if (is_white_space(letter)) {
      j++;
    } else {
      return 0;
    }
  }
  return 0;
}

void add_job(pid_t pid, char *name) {
  Node *node = createnode(pid, name);
  add(node, jobs);
}

void kill_job(Node *node) {
  pid_t pid = get_pid(node);

  del(node, jobs);

  printf("[KILL: %d]\n", pid);

  kill(pid, SIGQUIT);
  int status;
  for (;;) {
    pid_t child = wait(&status);
    if (child > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
      printf("child %d succesully quit\n", (int)child);
    } else if (child < 0 && errno == EINTR) {
      continue;
    } else {
      return;
    }
  }
}

/**
 *
 */
void kill_all_jobs(void) {
  foreach (jobs, kill_job)
    ;
}

/**
 * Method that takes in a char pointer and executes the program.
 */
int execute_command(CMDArg *args) {
  char *program = args->args[0];

  // exit
  if (strcmp(program, "exit") == 0) {
    kill_all_jobs();
    return EXIT;
  }

  // display jobs running in the background
  if (strcmp(program, "jobs") == 0) {
    display(jobs);
    return EXIT_SUCCESS;
  }

  // change current working directory of the shell
  if (strcmp(program, "cd") == 0) {
    change_directory(args->args[1]);
    return EXIT_SUCCESS;
  }

  if (is_background_command(args->args)) {
    // collect all background processes that have terminated (zombies) and print
    // their exit status

    // WEXITSTATUS(wstatus) returns the exit status of the child process
    // STAT_LOC

    for (Node *ptr = get_head(jobs); ptr != NULL; ptr = get_next(ptr)) {
      int status;
      pid_t pid = get_pid(ptr);
      if (waitpid(pid, &status, WNOHANG) > 0) {
        printf("Exit status [%s] = %d\n", get_name(ptr), WEXITSTATUS(status));
        del(ptr, jobs);
      }
    }
    // Exit status [/bin/echo test] = 0

    // run process in the background, and add it to the jobs list
    pid_t pid = fork();
    if (pid == 0) {
      execvp(args->args[0], args->args);
      printf("execvp(%d) error: %s", errno, strerror(errno));
      exit(EXIT_FAILURE);
    } else if (pid < 0) {
      perror("fork() error");
      return EXIT_FAILURE;
    } else {
      add_job(pid, args->args[0]);
      return EXIT_SUCCESS;
    }
  }

  // input and output redirection
  if (args->io_flag == 2) {
    // input and output redirection
    return i_o_ops(args->args, args->io_in_file, args->io_out_file,
                   args->append_flag);
  } else if (args->io_flag == 1) {
    // output redirection
    return output_ops(args->args, args->io_out_file, 0);
  } else if (args->io_flag == 0) {
    // input redirection
    return input_ops(args->args, args->io_in_file);
  } else {
    // no io operations
    return no_io_ops(args->args);
  }
}

int output_ops(char **cmd, char *output, int append_flag) {
  int exit_value;
  int flags;
  int stdout = dup(1);
  if (append_flag == 1) {
    flags = (O_RDWR | O_CREAT | O_APPEND);
  } else {
    flags = (O_RDWR | O_CREAT);
  }
  int file_fd = open(output, flags, 0640);
  if (file_fd == -1) {
    fprintf(stderr, "open(%d) error: %s\n", errno, strerror(errno));
    return errno;
  }
  print_char_pointer_pointer(cmd);
  dup2(file_fd, 1);
  // fork child process
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork");
    return errno;
  } else if (pid == 0) {
    execvp(cmd[0], cmd);
    fprintf(stderr, "execvp(%d) error: %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  } else {
    wait(&pid);
  }
  dup2(stdout, 1);
  close(stdout);
  close(file_fd);
  return EXIT_SUCCESS;
}

int input_ops(char **args, char *input) {
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
    printf("execvp(%d) error: %s", errno, strerror(errno));
    exit(EXIT_FAILURE);
  } else {
    wait(&pid);
  }
  dup2(stdin, 0);
  close(stdin);
  close(file_fd);
  return EXIT_SUCCESS;
}

int i_o_ops(char **cmd, char *input, char *output, int append_flag) {
  int exit_value;
  int flags;

  // set flags
  if (append_flag == 1) {
    flags = (O_RDWR | O_CREAT | O_APPEND);
  } else {
    flags = (O_RDWR | O_CREAT);
  }

  // manipulate fds
  int stdout = dup(1);
  int stdin = dup(0);
  int outfile_fd = open(output, flags, 0644);
  if (outfile_fd == -1) {
    perror("open out");
    return EXIT_FAILURE;
  }
  int infile_fd = open(input, O_RDONLY);
  if (infile_fd == -1) {
    perror("open in");
    return EXIT_FAILURE;
  }
  dup2(infile_fd, 0);
  dup2(outfile_fd, 1);

  // fork and execute cmd
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork");
    return EXIT_FAILURE;

  } else if (pid == 0) {
    execvp(cmd[0], cmd);
    perror("execvp");
    exit(1);
  }
  wait(&exit_value);

  // clean up
  dup2(stdin, 0);
  dup2(stdout, 1);
  close(stdin);
  close(stdout);
  close(infile_fd);
  close(outfile_fd);
  return EXIT_SUCCESS;
}

// https://stackoverflow.com/questions/18433585/kill-all-child-processes-of-a-parent-but-leave-the-parent-alive#answers
void sigquit_handler(int sig) {
  assert(sig == SIGQUIT);
  pid_t self = getpid();
  if (parent_pid != self) {
    _exit(EXIT_SUCCESS);
  }
}

// https://github.com/c-birdsey/mysh
int main(int argc, char *argv[]) {
  int status = 0;

  //   struct passwd *p = getpwuid(getuid());
  //   char *username = p->pw_name;
  //   char hostname[32];
  //   gethostname(hostname, sizeof(hostname));

  signal(SIGQUIT, sigquit_handler);
  parent_pid = getpid();
  jobs = makelist();
  char *line = NULL;
  CMDArg *cmd;
  update_cwd();
  while (status == 0) {
    // printf("%s@%s:%s$ ", username, hostname, cwd);
    printf("%s$ ", cwd);
    line = read_line(stdin);
    if (strcmp(line, "\n") == 0) {
      continue;
    }
    cmd = parse_args(line);
    status = execute_command(cmd);
  }
  destroy(jobs);
  printf("\nbye :)\n");
  return status == EXIT_SUCCESS;
}