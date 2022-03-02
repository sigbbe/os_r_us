/*
https://stackoverflow.com/questions/11785936/how-to-find-if-a-process-is-running-in-c?
https://stackoverflow.com/questions/6898337/determine-programmatically-if-a-program-is-running
*/
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

pid_t proc_find(const char *name) {
  DIR *dir;
  struct dirent *ent;
  char *endptr;
  char buf[512];

  if (!(dir = opendir("/proc"))) {
    perror("can't open /proc");
    return -1;
  }

  while ((ent = readdir(dir)) != NULL) {
    /* if endptr is not a null character, the directory is not
     * entirely numeric, so ignore it */
    long lpid = strtol(ent->d_name, &endptr, 10);
    if (*endptr != '\0') {
      continue;
    }

    /* try to open the cmdline file */
    snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);
    FILE *fp = fopen(buf, "r");

    if (fp) {
      if (fgets(buf, sizeof(buf), fp) != NULL) {
        /* check the first token in the file, the program name */
        char *first = strtok(buf, " ");
        if (!strcmp(first, name)) {
          fclose(fp);
          closedir(dir);
          return (pid_t)lpid;
        }
      }
      fclose(fp);
    }
  }

  closedir(dir);
  return -1;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("usage: %s name1 name2 ...\n", argv[0]);
    return 1;
  }
  for (int i = 1; i < argc; ++i) {
    pid_t pid = proc_find(argv[i]);
    printf("%u\n", pid);
    if (pid == -1) {
      printf("%s: not found\n", argv[i]);
    } else {
      printf("%s: %d\n", argv[i], pid);
    }
  }

  return 0;
}
