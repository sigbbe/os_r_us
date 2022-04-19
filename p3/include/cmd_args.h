#ifndef __CMD_ARGS_H
#define __CMD_ARGS_H

#define TOKEN_BUFSIZE 64

/**
 * Struct for agruements. Holds the char ** pointer to the list of arguments.
 * Also keeps track of io_flag, append_flag, number of arguments.
 */
typedef struct CMDArg {
  char **args;
  char *io_in_file;
  char *io_out_file;
  int io_flag;
  // io_flag = -1 => no redirect;
  // io_flag = 0 => input redirect;
  // io_flag = 1 => output redirect;
  // io_flag = 2 => input/output redirect;
  // io_flag = 3 => pipe;
  int append_flag;
  int arg_count;
} CMDArg;

/**
 * Function for initializing cmd_args struct
 */
CMDArg *init_cmd_args(void);

/**
 * Function for descructing cmd_args struct
 */
void destroy_cmd_args(CMDArg *args);

/**
 *
 * Function for adding another argument to the cmd_args.args array.
 * If the array is full realloc the array and add the new element to the end.
 *
 */
void add_arg(CMDArg *args, char *arg);

/**
 * Function for setting the io_flag. receives char pointer as argument, and
 * looks for '<' and '>' characters.
 * io_flag = -1 => no redirect;
 * io_flag = 0 => input redirect;
 * io_flag = 1 => output redirect;
 * io_flag = 2 => input/output redirect;
 */
void set_io_flag(CMDArg *args, char *arg);

/**
 * Setter function for io_file property of cmd_args struct
 */
void set_io_in_file(CMDArg *args, char *file);

/**
 * Setter function for io_file2 property of cmd_args struct
 */
void set_io_out_file(CMDArg *args, char *file);

#endif
