#include <stdio.h>
#include <stdlib.h>
#define __USE_XOPEN
#include <time.h>
#define MY_TIME_H
#include "../include/my_time.h"
#include <bits/types/time_t.h>
#include <string.h>
#include <time.h>

char *unix_timestamp_seconds_to_str(long int t) {
  const time_t *time = &t;
  return ctime(time);
}

struct tm tm_from_seconds(long int time) {
  struct tm tm;
  memcpy(&tm, localtime(&time), sizeof(struct tm));
  return tm;
}

int time_str(long int t, char buffer[]) {
  time_t rawtime = t;
  int ch;
  struct tm *info;
  info = localtime(&rawtime);
  ch = strftime(buffer, 80, "%H:%M:%S", info);
  return ch;
}

int date_str(long int t, char buffer[]) {
  time_t rawtime = t;
  int ch;
  struct tm *info;
  info = localtime(&rawtime);
  ch = strftime(buffer, 80, "%d.%m.%Y", info);
  return ch;
}

long int str_to_unix_timestamp_seconds(const char *__restrict time_str) {
  struct tm tm;
  strptime(time_str, "%Y-%m-%d %H:%M", &tm);
  tm.tm_isdst = 0;
  return mktime(&tm);
}

long int unix_timestamp_now(void) {
  time_t seconds;
  return time(&seconds);
}