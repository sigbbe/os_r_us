#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/cmd_args.h"

#define BUF 1024

/**
 * Function for initializing cmd_args struct
 */
CMDArg *init_cmd_args() {
  CMDArg *args = malloc(sizeof(CMDArg));
  args->args = malloc(sizeof(char *) * TOKEN_BUFSIZE);
  args->io_flag = -1;
  args->append_flag = 0;
  args->arg_count = 0;
  return args;
}

/**
 * Function for descructing cmd_args struct
 */
void destroy_cmd_args(CMDArg *args) {
  free(args->args);
  free(args);
}

/**
 *
 * Function for adding another argument to the cmd_args.args array.
 * If the array is full realloc the array and add the new element to the end.
 *
 */
void add_arg(CMDArg *args, char *arg) {
  if (args->arg_count >= TOKEN_BUFSIZE) {
    args->args = realloc(args->args, sizeof(char *) * (TOKEN_BUFSIZE * 2));
  }
  // remove trailing newline character
  char *newline = strchr(arg, '\n');
  if (newline)
    *newline = 0;
  args->args[args->arg_count++] = arg;
}

/**
 * Function for setting the io_flag. receives char pointer as argument, and
 * looks for '<' and '>' characters.
 * io_flag = -1 => no redirect;
 * io_flag = 0 => input redirect;
 * io_flag = 1 => output redirect;
 * io_flag = 2 => input/output redirect;
 */
void set_io_flag(CMDArg *args, char *arg) {
  if ((strchr(arg, '<')) && (strchr(arg, '>'))) {
    (*args).io_flag = 2;
  } else if (strchr(arg, '<')) {
    (*args).io_flag = 0;
  } else if (strchr(arg, '>')) {
    (*args).io_flag = 1;
  } else if (strstr(arg, ">>")) {
    (*args).io_flag = 0;
    (*args).append_flag = 1;
  } else if (strchr(arg, '|')) {
    (*args).io_flag = 3;
  } else {
    (*args).io_flag = -1;
  }
}

/**
 * Setter function for io_file property of cmd_args struct
 */
void set_io_in_file(CMDArg *args, char *file) {
  //   (*args).io_in_file = file;
  strcpy(args->io_in_file, file);
}

/**
 * Setter function for io_file2 property of cmd_args struct
 */
void set_io_out_file(CMDArg *args, char *file) {
  //   (*args).io_out_file = file;
  strcpy(args->io_out_file, file);
}

/**
* Check if the last letter of the input is a '&', and start a background process
if it is.
// */
// void set_background_flag(CMDArg *args) {
//   if (args->args[args->arg_count - 1][strlen(args->args[args->arg_count - 1])
//   - 1] == '&') {
//     args->args[args->arg_count - 1][strlen(args->args[args->arg_count - 1]) -
//     1] = 0; args->background_flag = 1;
//   } else {
//     args->background_flag = 0;
//   }
// } // end set_background_flag

/**
 * If the inut equals "jobs" print a list of all background processes.
 */
// void set_jobs_flag(CMDArg *args) {
//   if (strcmp(args->args[0], "jobs") == 0) {
//     args->jobs_flag = 1;
//   } else {
//     args->jobs_flag = 0;
//   }