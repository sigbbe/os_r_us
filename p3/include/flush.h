#ifndef __FLUSH_H
#define __FLUSH_H

#include "cmd_args.h"
#include <stdio.h>

#define INPUT_SIZE 4096
#define DELIM_CHARS " \t\r\n\a"
#define EXIT 1

/**
 * function for executing commands without any i/o operations
 */
int no_io_ops(char **args);

/**
 * function for executing commands containing input operations)
 */
int input_ops(char **args, char *input);

/**
 * function for executing commands containing output operations)
 */
int output_ops(char **cmd, char *output, int append_flag);

/**
 * function for executing commands containing input- and output operations
 */
int i_o_ops(char **cmd, char *input, char *output, int append_flag);

/**
 * Method that takes in a char pointer and executes the program.
 */
int execute_command(CMDArg *args);

#endif