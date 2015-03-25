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
#include <errno.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
int fcntl_struct(int file_des, int cmd, int type, off_t offset, int whence, off_t len){
	struct flock lock;

	lock.l_len = len;
	lock.l_start = offset;
	lock.l_type = type;
	lock.l_whence = whence;

	return fcntl(file_des, cmd, &lock);
}
void listLocks(int handle) {
  int end_of_loop = 0;
	char* lock_type;
	struct flock lock;
	lock.l_len = 0;
	lock.l_start = 0;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;

	while(!end_of_loop){
		if((fcntl(handle, F_GETLK, &lock)) == -1){
			printf("\nError in function list_locks() -> fcntl()\nMessage: \"%s\", errno: %d\n\n", strerror(errno), errno);
      return;
		}
		if(lock.l_type != F_UNLCK){
			if(lock.l_type == F_WRLCK)
				lock_type = "write-";
			else if(lock.l_type == F_RDLCK)
				lock_type = "read-";
			else if(lock.l_type == F_WRLCK)
				lock_type = "";

			printf("PID %d has %slocked byte %d\n", (int) lock.l_pid, lock_type, (int) lock.l_start);
			lock.l_len = 0;
			lock.l_start++;
			lock.l_whence = SEEK_SET;
		}
		else
			end_of_loop = 1;
	}
}
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
void writeLock(int handle) {
  long int byt;
  printf("Set byte number: ");
  scanf("%ld", &byt);
  if((fcntl_struct(handle, F_SETLK, F_WRLCK, byt, SEEK_SET, 1)) == -1){
		if(errno == EACCES || errno == EAGAIN)
			printf("It seems that another process has locked this byte\n"
					"type 'list_locks' to get more informations\n");
		else {
			printf("\nError in function w_lock() -> fcntl()\nMessage: \"%s\", errno: %d\n\n", strerror(errno), errno);
			return;
		}
  }
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
          writeLock(handle);
          break;
        case '3':
          listLocks(handle);
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
