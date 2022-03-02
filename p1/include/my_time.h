
#ifndef MY_TIME_H
#define MY_TIME_H

char *unix_timestamp_seconds_to_str(long int);

struct tm tm_from_seconds(long int time);

int date_str(long int i, char buffer[]);
int time_str(long int t, char buffer[]);

long int str_to_unix_timestamp_seconds(const char *);

long int unix_timestamp_now(void);

#endif /* MY_TIME_H */