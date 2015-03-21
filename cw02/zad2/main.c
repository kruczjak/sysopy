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

int main(int argc, char **argv) {
  if (argc!=3) exit_error(1, "Bad number of arguments (should be 2)\n");

  DIR *dir;
  struct dirent *dp;
  struct stat fileStat;
  char * path = argv[1];
  char * perm = argv[2];

  if (strlen(perm)!=3 || !check_char_int(perm)) exit_error(1, "Bad permissions\n");

  if ((dir = opendir(path)) == NULL) exit_error(1,"Cannot open dir\n");

  printf("File name:    \tFile size:\tTime:      \n");
  while ((dp = readdir(dir)) != NULL) {
    char * tmp;
    if (path[strlen(path)-1]!='/') tmp = concat(path,"/", dp->d_name);
    else tmp = concat(path,"", dp->d_name);

    if (lstat(tmp,&fileStat) == -1) {
      free(tmp);
      continue;
    }
    free(tmp);

    if(!S_ISREG(fileStat.st_mode) || !perm_cmp(atoi(perm), fileStat.st_mode)) continue;

    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&fileStat.st_mtime));
    printf("%s\t%dB\t%s\n", dp->d_name, (int) fileStat.st_size, buf);
  }

  closedir(dir);
  return 0;
}
