#define _XOPEN_SOURCE
#include "../include/my_time.h"
#include <alloca.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h> // for open
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h> // for close

#define NUM_ALARMS 3

typedef int bool;
#define true 1
#define false 0

enum alarmclock_actions {
  SCHEDULE = 0,
  LIST,
  CANCEL,
  EXIT,
  NO_ACTION,
};

static const char *const actions[4] = {
    [SCHEDULE] = "s", [LIST] = "l", [CANCEL] = "c", [EXIT] = "e"};

struct Alarm {
  int pid;
  int t_time;
};

void welcome(void) {
  char w[150];
  strcpy(w, "\nWelcome to the alarm clock! It is currently ");
  char t[32];
  FILE *f = popen("date '+%d-%m-%Y %H:%M:%S'", "r");
  fgets(t, sizeof(t), f);
  pclose(f);
  strcat(w, t);
  strcat(w, "Please enter \"s\"(schedule), \"l\"(list), \"c\"(cancel), "
            "\"x\"(exit)\n");
  printf("%s\n", w);
  return;
}

int i_round(int a, int b) {
  return a % b >= (b / 2) ? a + b - a % b : a - a % b;
}

time_t schedule_alarm_menu() {
  char *date = malloc(32);
  char *time = malloc(32);
  printf("%s", "Schedule alarm (DD-MM-YYYY HH:MM:SS): ");
  scanf("%s %s", date, time);
  strcat(date, " ");
  strcat(date, time);
  return str_to_unix_timestamp_seconds(date);
}

int press_to_continue(void) {
  printf("%s", "\n");
  system("read -p 'Press Enter to continue...' var");
  return 0;
}

void watch_alarm(int time) {
  int now = unix_timestamp_now();
  int sleep_time = time - now;
  sleep(sleep_time);
  printf("\nALARM!\nALARM!\nALARM!");
  char *args[] = {"/bin/mpg123", "-q", "/home/sigbbe/Desktop/harry_maguire.mp3",
                  NULL};
  execvp("mpg123", args);
  return;
}

void schedule(struct Alarm alarm[], int num_alarms, long int new_alarm_time) {
  for (int i = 0; i < num_alarms; i++) {
    if (alarm[i].pid == 0 || alarm[i].t_time == 0) {
      int pid = fork();
      alarm[i].t_time = new_alarm_time;
      if (pid != 0) {
        alarm[i].pid = pid;
      } else {
        alarm[i].pid = getpid();
        watch_alarm(alarm[i].t_time);
      }
      return;
    }
  }
  printf("%s\n", "Cannot create more alarms");
}
struct Alarm new_alarm(void) {
  struct Alarm alarm;
  alarm.pid = 0;
  alarm.t_time = 0;
  return alarm;
}

bool is_alarm_unset(struct Alarm *alarm) {
  return alarm->pid == 0 && alarm->t_time == 0;
}

bool has_active_alarm(struct Alarm alarm[], int start, int end) {
  if (start + 1 >= end) {
    return !is_alarm_unset(&alarm[start]);
  } else {
    return !is_alarm_unset(&alarm[start]) ||
           has_active_alarm(alarm, start + 1, end);
  }
}

void list(struct Alarm alarm[], int len) {
  //   system("clear");
  if (!has_active_alarm(alarm, 0, len)) {
    printf("%s\n", "You have no active alarms");
    return;
  }
  //   system("clear");
  printf("PID\t\tAlarm\t\t\t\tID\n");
  printf("_________________________________________________\n");
  for (int i = 0; i < len; i++) {
    // printf("%d\n", alarm[i].pid != 0 && alarm[i].t_time != 0);
    if (!is_alarm_unset(&alarm[i])) {
      int time = alarm[i].t_time;
      int *time_copy = malloc(sizeof &time);
      *time_copy = time;
      char date_buf[12];
      date_str(alarm[i].t_time, date_buf);
      char time_buf[12];
      time_str(alarm[i].t_time, time_buf);
      printf("%d\t\t%s %s\t\t%d\n", alarm[i].pid, date_buf, time_buf, i);
    }
  }
}

int cancel_alarm_menu(struct Alarm alarm[], int len) {
  for (int i = 0; i < len; i++) {
    int time = alarm[i].t_time;
    if (alarm[i].pid != 0 && time != 0) {
      char *date_buf = malloc(16);
      char *time_buf = malloc(16);
      date_str(time, date_buf);
      strcat(date_buf, " ");
      time_str(time, time_buf);
      strcat(date_buf, time_buf);
      date_buf[strcspn(date_buf, "\n")] = 0;
      char ans[1];
      printf("\nCancel alarm scheduled for %s (y/N): ", date_buf);
      scanf(" %c", ans);
      if (strcmp(ans, "y") == 0) {
        kill(alarm[i].pid, SIGKILL);
        alarm[i] = new_alarm();
      }
      free(time_buf);
      free(date_buf);
    }
  }
  welcome();
  return 0;
  // Funker å slette før alarmen går, men er litt bugga etter...
}

int main(int argc, char **argv) {
  struct Alarm alarms[NUM_ALARMS];
  // initialize unset alarms
  for (int i = 0; i < NUM_ALARMS; i++) {
    alarms[i] = new_alarm();
  }
  welcome();
  char choice;
  int i = 0;
  while (1) {
    int status;
    for (int i = 0; i < NUM_ALARMS; i++) {
      if (0 < waitpid(alarms[i].pid, &status, WNOHANG)) {
        alarms[i] = new_alarm();
        // welcome();
      }
    }
    // system("clear");
    printf("> ");
    int scan = scanf("%s", &choice);
    choice = tolower(choice);
    if (0 == strcmp(&choice, actions[SCHEDULE])) {
      long int new_alarm_time = schedule_alarm_menu();
      schedule(alarms, NUM_ALARMS, new_alarm_time);
      printf("Scheduling alarm in %ld seconds\n",
             (new_alarm_time - unix_timestamp_now()));
      welcome();
    } else if (0 == strcmp(&choice, actions[LIST])) {
      list(alarms, NUM_ALARMS);
      welcome();
      fflush(stdout);
    } else if (0 == strcmp(&choice, actions[CANCEL])) {
      list(alarms, NUM_ALARMS);
      int remove = cancel_alarm_menu(alarms, NUM_ALARMS);
      welcome();
      fflush(stdout);
    } else if (0 == strcmp(&choice, actions[EXIT]) ||
               0 == strcmp(&choice, "q") || 0 == strcmp(&choice, "x")) {
      printf("\nBYE :)\n");
      break;
    }
  }
  return 0;
}

// gcc -std=gnu99 -o main src/lib.c src/alarmclock.c