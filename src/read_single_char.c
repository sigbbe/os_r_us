

// use recursion function
void revstr(char *str1) {
  // declare static variable
  static int i, len, temp;
  len = strlen(str1); // use strlen() to get the length of str string

  if (i < len / 2) {
    // temp variable use to temporary hold the string
    temp = str1[i];
    str1[i] = str1[len - i - 1];
    str1[len - i - 1] = temp;
    i++;
    revstr(str1); // recusively calls the revstr() function
  }
}

void foo(void) {
  //   revstr(buffer);
  //   printf("%s\n+", buffer);
  //   buffer[strcspn(buffer, "\n")] = 0;
  //   printf("%s\n-", buffer);
  //   fp = fopen(addr, "w");
  //   if (fp) {
  //     fputs(NULL, fp);
  //     fclose(fp);
  //   }
}

char *stdout_path(char buf[]) {
  buf = "/proc/";
  char pid[32];
  sprintf(pid, "%d", getpid());
  strcat(buf, pid);
  strcat(buf, "/fd/1");
  printf("%s\n", buf);
  return buf;
}

int write_stdout(char *buf) {
  char *addr = stdout_path(addr);
  printf("%s\n", addr);
  //   FILE *fp = fopen(addr, "w");
  //   char *buffer = malloc(10);
  //   if (fp != 0) {
  //     fputs(buf, fp);
  //     fclose(fp);
  //   }
  return 0;
}

int delete_last_line_stdout(void) {
  // 1. read entire file
  // 2. reverse file contents
  // 3. delete end-second \n
  // 4. reverse file contents
  return 0;
}

int main(int argc, char **argv) {
  char buf[] = "\nbaiselure\nbaiselure\nbaiselure\nbaiselure";
  write_stdout(buf);
  //   revstr(buf);
  //   printf("%s", buf);
  return 0;
}

// gcc -std=c99 -Wall src/lib.c src/read_single_char.c -o main