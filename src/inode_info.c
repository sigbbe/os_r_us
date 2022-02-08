#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc >= 2) {
    for (int i = 0; i < argc; i++) {
      struct stat fileStat;
      int fd = 0;
      FILE *filename = argv[i];

      if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("open ");
        system("pause");
        exit(1);
      }

      if (fstat(fd, &fileStat) < 0)
        return 1;

      printf("Information for %s\n", filename);
      printf("---------------------------\n");
      printf("File Size: \t\t%d bytes\n", fileStat.st_size);
      printf("Number of Links: \t%d\n", fileStat.st_nlink);
      printf("File inode: \t\t%d\n", fileStat.st_ino);
      printf("\n");
    }
  }
  system("read -p 'Press Enter to continue...' var");
  return 0;
}