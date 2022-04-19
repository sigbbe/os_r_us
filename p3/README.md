# Fush - Basic unix shell

## Basic functionality

TODO:

1. [x] Current working directory followed by a colon
2. [x] The entered text is split into command name and arguments
3. [x] Create a new process (using fork(2))
4. [x] Execute the entered command with the given parameters (using a variant of exec(3))

## Changing directories

**Why does `cd` have to be an internal shell-command?**

The cd command must be builtin because the shell itself needs to change its _current working directory_. Changing the actual current working directory of the shell from a child process is more difficult than making the command itself built into the shell.

TODO:

1. [x] Implement cd command with chdir(2).
2. [x] cd command has to be internal.
3. [x] Empty cd commands can be ignored.

## I/O redirection

TODO:

- [x] Test cases
  1. [x] ls > /tmp/foo
  2. [x] head -1 < /etc/passwd
  3. [x] head -1 < /etc/passwd > /tmp/foo2

PROG > FILE: redirects the stdout of PROG to FILE. Overwrite FILE if it already exists.
PROG &>FILE: redirects both the stdout and the stderr of PROG to FILE.

## Background task status

TODO:

- [x] If a command line ends with the character "&"
- [x] Linked list
- [x] Executed as a background process

## Optional bonus task: Pipelines
