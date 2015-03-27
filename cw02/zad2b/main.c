#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

int perm_int = 0;

bool perm_cmp(int perm, mode_t st_mode) {
  char mode[7];
  snprintf(mode, 8, "%o", st_mode); //printf number to mode with "%o"
  int mode_int = atoi(mode);
  mode_int %= 1000;
	return (perm == mode_int) ? true : false;
}

bool check_char_int(char * to_check) {
  for (int i=0; i < strlen(to_check); i++)
    if(to_check[i] < '0' || to_check[i] > '7')
      return false;
  return true;
}
void exit_error(int type, char * message) {
  printf(message);
  exit(type);
}
char * concat(char * first, char * middle, char * last) {
  char * ret = (char *) malloc(1+strlen(first)+strlen(middle)+strlen(last));
  strcpy(ret, first);
  strcat(ret, middle);
  strcat(ret, last);
  return ret;
}

static int display_info(const char* fpath, const struct stat *sb, int typeFlag) {
  if(!S_ISREG(sb->st_mode) || !perm_cmp(perm_int, sb->st_mode)) return 0;
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&sb->st_mtime));
  printf("%s\t%dB\t%s\n", fpath, (int) sb->st_size, buf);
  return 0;
}

int main(int argc, char **argv) {
  if (argc!=3) exit_error(1, "Bad number of arguments (should be 2) main.run <path> <permission>\n");

  char * path = argv[1];
  char * perm = argv[2];
  if (strlen(perm)!=3 || !check_char_int(perm)) exit_error(1, "Bad permissions\n");
  perm_int = atoi(perm);
  printf("File path:    \tFile size:\tTime:      \n");
  if (ftw(path, display_info, 1) == -1) exit_error(1,"FTW error\n");

  return 0;
}
