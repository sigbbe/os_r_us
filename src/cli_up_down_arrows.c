#include "../include/my_time.h"
#define MY_TIME_H
#define _XOPEN_SOURCE
#include <aio.h>
#include <ctype.h>
#include <pthread.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

char *menu_str(long int time) {
  char *msg = "Schedule alarm at which date and time? ";
  char *time_str = unix_timestamp_seconds_to_str(time);
  strcat(msg, time_str);
  return msg;
}

// get state of stdin

void *thread_func(void *argument) {
  int lim = 0;
  while (1) {
    int c = getchar();
    printf("%d\n", c);
    if (c == 10) {
      if (lim) {
        break;
      } else {
        lim++;
      }
    } else {
      lim = 0;
    }
  }
  return NULL;
}

void foo(void) {
  //   close(STDIN_FILENO);
  //   pthread_t thread;
  //   void **retval = NULL;
  //   pthread_join(thread, retval); // or whatever to wait for thread exit
  //   system("reset -Q");           // -Q to avoid displaying cruf
  //
  pthread_t my_thread;
  int a = 10;
  pthread_create(&my_thread, NULL, thread_func, &a); // no parentheses here
  pthread_join(my_thread, NULL);
}

int main(int argc, char **argv) {
  //   //   long int time = unix_timestamp_now();
  //   //   char *time_str = unix_timestamp_seconds_to_str(time);
  //   char choice;
  //   int i = 0;
  //   while (1) {
  //     // printf("> ");
  //     char *msg = "Schedule alarm at which date and time? ";
  //     int scan = scanf("%s", &choice);
  //     readline("Bais");
  //     // choice = tolower(choice);
  //     // time = unix_timestamp_now();
  //     // time_str = unix_timestamp_seconds_to_str(time);
  //     break;
  //   }
  return 0;
}
