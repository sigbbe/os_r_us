#define _XOPEN_SOURCE
#include "../include/my_time.h"
#include <alloca.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define NUM_ALARMS 3

typedef int bool;
#define true 1
#define false 0

enum alarmclock_actions {
  SCHEDULE = 0,
  LIST,
  CANCEL,
  EXIT,
  HELP,
  CLEAR,
  NO_ACTION,
};

static const char *const actions[6] = {
    [SCHEDULE] = "s", [LIST] = "l", [CANCEL] = "c",
    [EXIT] = "e",     [HELP] = "h", [CLEAR] = "r",
};

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
            "\"e\"(exit), \"h\"(help), \"r\"(clear)\n");
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

void watch_alarm(int time) {
  int now = unix_timestamp_now();
  int sleep_time = time - now;
  sleep(sleep_time);
  printf("\nALARM!\nALARM!\nALARM!\n\n");
  char *args[] = {"/bin/mpg123", "-q", "./harry_maguire.mp3", NULL};
  execvp(args[0], args);
  fprintf(stderr, "\nChild process could play alarm sound!\n");
  exit(1);
}

void schedule(struct Alarm alarm[], int num_alarms, long int new_alarm_time) {
  if (new_alarm_time < unix_timestamp_now()) {
    printf("\nAlarm time must be in the future!\n");
    return;
  }
  for (int i = 0; i < num_alarms; i++) {
    if (alarm[i].pid == 0 || alarm[i].t_time == 0) {
      printf("Scheduling alarm in %ld seconds\n",
             (new_alarm_time - unix_timestamp_now()));
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
  if (!has_active_alarm(alarm, 0, len)) {
    printf("%s\n", "You have no active alarms");
    return;
  }
  printf("PID\t\tAlarm\t\t\t\tID\n");
  printf("_________________________________________________\n");
  for (int i = 0; i < len; i++) {
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
        printf("Removed alarm: %d\n", i);
      }
      free(time_buf);
      free(date_buf);
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  struct Alarm alarms[NUM_ALARMS];
  // initialize unset alarms
  for (int i = 0; i < NUM_ALARMS; i++) {
    alarms[i] = new_alarm();
  }
  welcome();
  char choice[5];
  while (1) {
    printf("%s", "> ");
    scanf(" %*[ ]");
    scanf("%[^ \n]%*[ ]", choice);
    int status;
    for (int i = 0; i < NUM_ALARMS; i++) {
      int now = unix_timestamp_now();
      if (0 < waitpid(alarms[i].pid, &status, WNOHANG) ||
          alarms[i].t_time < now) {
        alarms[i] = new_alarm();
      }
    }
    if (0 == strcmp(choice, actions[SCHEDULE])) {
      long int new_alarm_time = schedule_alarm_menu();
      schedule(alarms, NUM_ALARMS, new_alarm_time);
    } else if (0 == strcmp(choice, actions[LIST])) {
      list(alarms, NUM_ALARMS);
    } else if (0 == strcmp(choice, actions[CANCEL])) {
      list(alarms, NUM_ALARMS);
      cancel_alarm_menu(alarms, NUM_ALARMS);
    } else if (0 == strcmp(choice, actions[HELP])) {
      welcome();
    } else if (0 == strcmp(choice, actions[CLEAR])) {
      system("clear");
    } else if (0 == strcmp(choice, actions[EXIT])) {
      for (int i; i < NUM_ALARMS; i++) {
        if (alarms[i].pid != 0) {
          kill(alarms[i].pid, SIGKILL);
        }
      }
      printf("\nBYE :)\n");
      break;
    } else {
      printf("Type a valid character, type 'h' for help.\n");
    }
  }
  return 0;
}

// gcc -std=gnu99 -o main src/lib.c src/alarmclock.c