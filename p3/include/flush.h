#ifndef __FLUSH_H
#define __FLUSH_H

#include "cmd_args.h"
#include <stdio.h>

#define INPUT_SIZE 4096
#define DELIM_CHARS " \t\r\n\a"

/**
 * function for executing commands without any i/o operations
 */
int no_io_ops(char **args);

/**
 * function for executing commands containing input operations)
 */
int i_ops(char **args, char *input);

/**
 * function for executing commands containing output operations)
 */
int o_ops(char **cmd, char *output, int append_flag);

/**
 * function for executing commands containing input- and output operations
 */
int io_ops(char **args);

/**
 * Method that takes in a char pointer and executes the program.
 */
int execute_command(CMDArg *args);

#endif