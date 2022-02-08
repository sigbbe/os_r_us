#define _XOPEN_SOURCE
#include "../include/my_time.h"
#include <alloca.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h> // for open
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h> // for close
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
  strcpy(w, "Welcome to the alarm clock! It is currently ");
  char t[32];
  FILE *f = popen("date '+%Y-%m-%d %H:%M:%S'", "r");
  fgets(t, sizeof(t), f);
  pclose(f);
  strcat(w, t);
  strcat(w, "Please enter \"s\"(schedule), \"l\"(list), \"c\"(cancel), "
            "\"x\"(exit)\n");
  printf("%s\n", w);
  return;
}

char getch() {
  char buf = 0;
  struct termios old = {0};
  if (tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0)
    perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0)
    perror("read()");
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0)
    perror("tcsetattr ~ICANON");
  return (buf);
}

int i_round(int a, int b) {
  return a % b >= (b / 2) ? a + b - a % b : a - a % b;
}

long int schedule_alarm_menu() {
  system("clear");
  int c;
  long int now = unix_timestamp_now();
  long int time = i_round(unix_timestamp_now(), 60);
  char buf[10];
  date_str(time, buf);
  printf("%s %s\n", "Schedule alarm at which date? (+/-)", buf);
  while ((c = getch()) != '\n' && c != EOF) {
    printf("%d\n", c);
    if (c == 45) {
      if (time - 86400 > now) {
        time -= 86400;
      }
    } else if (c == 43) {
      time += 86400;
    }
    date_str(time, buf);
    system("clear");
    printf("%s %s\n", "Schedule alarm at which date? (+/-)", buf);
  }
  system("clear");
  time_str(time, buf);
  printf("%s %s\n", "Schedule alarm at which time? (+/-)", buf);
  while ((c = getch()) != '\n' && c != EOF) {
    printf("%d\n", c);
    if (c == 45) {
      if (time - 60 > now) {
        time -= 60;
      }
    } else if (c == 43) {
      time += 60;
    }
    time_str(time, buf);
    system("clear");
    printf("%s %s\n", "Schedule alarm at which time? (+/-)", buf);
  }
  return time;
}

int press_to_continue(void) {
  printf("%s", "\n\n");
  system("read -p 'Press Enter to continue...' var");
  return 0;
}

void watch_alarm(struct Alarm *alarm) {
  printf("%d\n", alarm->pid);
  printf("%d\n", alarm->t_time);
  return;
}

void schedule(struct Alarm alarm[], int num_alarms, int new_alarm_time) {
  for (int i = 0; i < num_alarms; i++) {
    if (alarm[i].pid == 0 || alarm[i].t_time == 0) {
      pthread_t thread_id = 1;
      alarm[i].pid = thread_id;
      alarm[i].t_time = new_alarm_time;
      //   pthread_create(&thread_id, NULL, (void *)&watch_alarm, &alarm[i]);
      return;
    }
  }
  printf("%s\n", "Cannot create more alarms");
}

void reset_alarm(struct Alarm alarm) {
  alarm.pid = 0;
  alarm.t_time = 0;
}

struct Alarm new_alarm(void) {
  struct Alarm alarm;
  alarm.pid = 0;
  alarm.t_time = 0;
  return alarm;
}

bool is_alarm_unset(struct Alarm *alarm) {
  printf("%d %d\n", alarm->pid, alarm->t_time);
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
  system("clear");
  if (!has_active_alarm(alarm, 0, len)) {
    printf("%s\n", "You have no active alarms");
    return;
  }
  system("clear");
  printf("PID\t\tAlarm\n");
  printf("___________________________________\n");
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
      printf("%d\t\t%s %s\n", alarm[i].pid, date_buf, time_buf);
    }
  }
}

int cancel_alarm_menu(struct Alarm alarm[], int len) {
  for (int i = 0; i < len; i++) {
    if (alarm[i].pid != 0 && alarm[i].t_time != 0) {
      char *alarm_buf = unix_timestamp_seconds_to_str(alarm[i].pid);
      printf("Cancel alarm scheduled for %s (y/N)\n", alarm_buf);
      char ans = getch();
      if (strcmp(&ans, "y") == 0) {
        reset_alarm(alarm[i]);
      }
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  struct Alarm alarms[NUM_ALARMS];
  // initialize unset alarms
  for (int i = 0; i < 3; i++) {
    alarms[i] = new_alarm();
  }
  char choice;
  int i = 0;
  while (1) {
    system("clear");
    welcome();
    printf("> ");
    int scan = scanf("%s", &choice);
    choice = tolower(choice);
    if (0 == strcmp(&choice, actions[SCHEDULE])) {
      int new_alarm_time = schedule_alarm_menu();
      schedule(alarms, NUM_ALARMS, new_alarm_time);
      printf("Scheduling alarm in %ld seconds\n",
             (new_alarm_time - unix_timestamp_now()));
      press_to_continue();
    } else if (0 == strcmp(&choice, actions[LIST])) {
      list(alarms, NUM_ALARMS);
      press_to_continue();
      fflush(stdout);
    } else if (0 == strcmp(&choice, actions[CANCEL])) {
      list(alarms, NUM_ALARMS);
      int remove = cancel_alarm_menu(alarms, NUM_ALARMS);
      if (remove >= 0 && remove < NUM_ALARMS) {
        char *tmp_alarm = unix_timestamp_seconds_to_str(alarms[remove].t_time);
        alarms[remove].pid = 0;
        alarms[remove].t_time = 0;
        printf("Removed alarm for %s\n", tmp_alarm);
      }
      press_to_continue();
      fflush(stdout);
    } else if (0 == strcmp(&choice, actions[EXIT]) ||
               0 == strcmp(&choice, "q") || 0 == strcmp(&choice, "x")) {
      printf("\nBYE :)\n");
      break;
    }
  }
  return 0;
}