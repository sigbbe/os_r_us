void myfunc(char *const src, int len) {
  int i;
  for (i = 0; i < len; ++i) {
    src[i] = 42;
  }
}

int main(void) {
  char arr[] = {'a', 'b', 'c', 'd'};
  int len = sizeof(arr);
  myfunc(arr, len);
  myfunc(arr, len + 1);
  myfunc(arr, len);
  return 0;
}

// gcc -fstack-protector-all -g -O0 -std=c99 stack_smash.c
// gcc -fsanitize=address -fstack-protector-all -g -O0 -std=c99 stack_smash.c