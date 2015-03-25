#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"

void readByte(int handle) {
  char buf;
  long int byt;
  printf("Set byte number: ");
  scanf("%ld", &byt);
  lseek(handle, byt, SEEK_SET);
  read(handle, &buf, 1);
  printf("%sByte %ld = %c\n\n", KGRN, byt, buf);
}
void writeByte(int handle) {
  char buf;
  long int byt;
  printf("Set byte number: ");
  scanf("%ld", &byt);
  lseek(handle, byt, SEEK_SET);
  printf("Char to write: ");
  scanf(" %c", &buf);
  write(handle, &buf, 1);
  printf("%sByte %ld = %c\n\n", KGRN, byt, buf);
}
void clearScreen()
{
  system("clear");
}
void exit_error(int type, char * message) {
  printf("%s%s%s", KRED, message, KNRM);
  exit(type);
}
void print_menu() {

  printf("%sHello :)\n\n1.Set read lock on byte\n2.Set write lock on byte\n3.List locked bytes\n"
  "4.Remove lock\n5.Read byte\n6.Write byte\n\nChoose option: ", KNRM);
}
int main(int argc, char **argv) {
  if (argc!=2) exit_error(1, "Bad number of arguments (should be 1)\n");
  int handle = open(argv[1], O_RDWR);
  if(handle < 0)
        exit_error(1, "Cannot open file\n");

  char input;
  clearScreen();

  while (input != 'q') {
    print_menu();
    while ((scanf(" %c", &input)!=1) || (input != 'q' && (input < '1' || input > '6'))) {
      printf("%sBad input, try again: %s", KRED, KNRM);
      continue;
    }
    switch (input) {
        case '1':
          break;
        case '2':
          break;
        case '5':
          readByte(handle);
          break;
        case '6':
          writeByte(handle);
          break;
        default:
          exit(1);
    }
  }


  return 0;
}
