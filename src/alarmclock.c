#define _XOPEN_SOURCE
#define _GNU_SOURCE
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

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#define BUF_SIZE 1024

// https://ofstack.com/C++/9293/linux-gets-pid-based-on-pid-process-name-and-pid-of-c.html
void get_pid_by_name(char *task_name, int pids[], int len) {
  DIR *dir;
  struct dirent *ptr;
  FILE *fp;
  char filepath[50]; // The size is arbitrary, can hold the path of cmdline file
  char cur_task_name[50]; // The size is arbitrary, can hold to recognize the
                          // command line text
  char buf[BUF_SIZE];
  dir = opendir("/proc"); // Open the path to the
  int i = 0;
  int limit = 0;
  if (NULL != dir) {
    while ((ptr = readdir(dir)) !=
           NULL) // Loop reads each file/folder in the path
    {
      // If it reads "." or ".." Skip, and skip the folder name if it is not
      // read
      if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
        continue;
      if (DT_DIR != ptr->d_type)
        continue;
      sprintf(filepath, "/proc/%s/status",
              ptr->d_name);      // Generates the path to the file to be read
      fp = fopen(filepath, "r"); // Open the file
      if (NULL != fp) {
        if (fgets(buf, BUF_SIZE - 1, fp) == NULL) {
          fclose(fp);
          continue;
        }
        sscanf(buf, "%*s %s", cur_task_name);
        // Print the name of the path (that is, the PID of the process) if the
        // file content meets the requirement
        if (!strcmp(task_name, cur_task_name)) {
          //   printf("%s\n", cur_task_name);
          //   printf("%d\n", i);
          pids[i++] = atoi(ptr->d_name);
          limit = 0;
        }
        fclose(fp);
        limit++;
      }
    }
    closedir(dir); // Shut down the path
  }
}

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
  time_t t_time;
};

struct Alarm new_alarm(void) {
  struct Alarm alarm;
  alarm.pid = 0;
  alarm.t_time = 0;
  return alarm;
}

bool alarm_valid_time(struct Alarm *alarm) {
  time_t now = unix_timestamp_now();
  return alarm->t_time > now && now + 94608000 > alarm->t_time;
}

bool is_alarm_unset(struct Alarm *alarm) {
  return alarm->pid == 0 && alarm->t_time == 0;
}

bool alarm_exists(struct Alarm *alarm) {
  int status;
  return 0 < waitpid(alarm->pid, &status, WNOHANG) && alarm_valid_time(alarm);
}

struct Alarm parse_alarm(char *line) {
  // line: PID:<pid>,TIME:<unix timestamp>
  char *ptr = line;
  struct Alarm alarm = new_alarm();
  while (*ptr) { // While there are more characters to process...
    if (isdigit(*ptr)) {
      // Found a number
      long int val = (long int)strtol(ptr, &ptr, 10); // Read number
      if (alarm.pid == 0) {
        alarm.pid = val;
      } else if (alarm.t_time == 0) {
        alarm.t_time = val;
      } else {
        break;
      }
    } else {
      // Otherwise, move on to the next character.
      ptr++;
    }
  }
  return alarm;
}

int i_round(int a, int b) {
  return a % b >= (b / 2) ? a + b - a % b : a - a % b;
}

bool has_active_alarm(struct Alarm alarm[], int start, int end) {
  if (start + 1 >= end) {
    return !(alarm_valid_time(&alarm[start]) && alarm_exists(&alarm[start]));
  } else {
    return !(alarm_valid_time(&alarm[start]) && alarm_exists(&alarm[start])) ||
           has_active_alarm(alarm, start + 1, end);
  }
}

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

void schedule_from_alarm(struct Alarm *alarm) {
  int pid = fork();
  if (pid != 0) {
    alarm->pid = pid;
  } else {
    watch_alarm(alarm->t_time);
  }
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

void list(struct Alarm alarm[], int len) {
  if (!has_active_alarm(alarm, 0, len)) {
    printf("%s\n", "You have no active alarms");
    return;
  }
  printf("PID\t\tAlarm\t\t\t\tID\n");
  printf("_________________________________________________\n");
  for (int i = 0; i < len; i++) {
    if (alarm_valid_time(&alarm[i])) {
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

int interactive() {
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

int read_alarms(char *filename, struct Alarm *items, int k) {
  FILE *fileptr;
  char *buffer;
  long filelen;
  const char separator[3] = "\r\n";
  char *line = NULL;
  int i = 0;

  for (int j = 0; j < k; j++) {
    items[j] = new_alarm();
  }

  fileptr = fopen(filename, "rb"); // Open the file in binary mode
  fseek(fileptr, 0, SEEK_END);     // Jump to the end of the file
  filelen = ftell(fileptr);        // Get the current byte offset in the file
  rewind(fileptr);                 // Jump back to the beginning of the file

  buffer = (char *)malloc(filelen * sizeof(char)); // Enough memory for the file
  fread(buffer, filelen, 1, fileptr);              // Read in the entire file
  // -------------------------------
  line = strtok(buffer, separator);
  while (line != NULL && i < k) {
    struct Alarm tmp_alarm = parse_alarm(line);
    if (alarm_valid_time(&tmp_alarm)) {
      items[i] = tmp_alarm;
      if (alarm_exists(&items[i]) == 0) {
        schedule_from_alarm(&items[i]);
      }
    }
    i++;
    line = strtok(NULL, separator); // NULL means continue from where the
  }
  // -------------------------------
  fclose(fileptr); // Close the file
  return 0;
}

int write_alarms(struct Alarm *alarms, int n) {
  //   FILE *pFile;
  //   char *yourFilePath = "./alarmclock.txt";
  //   char *yourBuffer = malloc(100);

  //   for (int i = 0; i < n; i++) {
  //     char *alarm_str;
  //     sprintf(alarm_str, "PID:%d,TIME:%ld", alarms[i].pid,
  //     alarms[i].t_time); strcat(yourBuffer, alarm_str);
  //   }

  //   int yorBufferSize = strlen(yourBuffer) + 1;

  //   /* Reserve memory for your readed buffer */
  //   char *readedBuffer = malloc(yorBufferSize);

  //   if (readedBuffer == 0) {
  //     puts("Can't reserve memory for Test!");
  //   }

  //   /* Write your buffer to disk. */
  //   pFile = fopen(yourFilePath, "wb");
  //   if (pFile) {
  //     fwrite(yourBuffer, yorBufferSize, 1, pFile);
  //   }
  //   return fclose(pFile);
  return 0;
}

void kill_alarms(int argc, char **argv) {
  int *pids = malloc(sizeof(int) * 10);
  char *program = malloc(16);
  strcpy(program, argv[0]);
  program += 2;
  get_pid_by_name(program, pids, 10);
  for (int i = 0; i < 10; i++) {
    if (pids[i] != 0 && pids[i] != getpid()) {
      kill(pids[i], SIGKILL);
    }
  }
}

int with_args(int argc, char **argv) {
  kill_alarms(argc, argv);
  struct Alarm alarms[NUM_ALARMS];
  int tmp_alarms = read_alarms("./alarmclock.txt", alarms, NUM_ALARMS);
  if (0 == strcmp(&argv[1][0], actions[SCHEDULE]) && argc == 4) {
    char time_str[20];
    strcpy(time_str, argv[2]);
    strcat(time_str, " ");
    strcat(time_str, argv[3]);
    time_t new_alarm_time = str_to_unix_timestamp_seconds(time_str);
    schedule(alarms, NUM_ALARMS, new_alarm_time);
    return 0;
  }
  if (argc != 2) {
    printf("Type a valid character, type 'h' for help.\n");
    return 0;
  }
  if (0 == strcmp(&argv[1][0], actions[LIST])) {
    list(alarms, NUM_ALARMS);
  } else if (0 == strcmp(&argv[1][0], actions[CANCEL])) {
    list(alarms, NUM_ALARMS);
    cancel_alarm_menu(alarms, NUM_ALARMS);
  } else if (0 == strcmp(&argv[1][0], actions[HELP])) {
    welcome();
  } else if (0 == strcmp(&argv[1][0], actions[CLEAR])) {
    system("clear");
  } else if (0 == strcmp(&argv[1][0], actions[EXIT])) {
    printf("\nBYE :)\n");
  } else {
    printf("Type a valid character, type 'h' for help.\n");
  }
  return write_alarms(alarms, NUM_ALARMS);
}

int main(int argc, char **argv) {
  if (argc == 1) {
    return interactive();
  } else if (argc >= 2) {
    return with_args(argc, argv);
  }
  return 1;
}

// gcc -std=gnu99 -o main src/lib.c src/alarmclock.c