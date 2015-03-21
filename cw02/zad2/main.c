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

bool check_char_int(char * to_check) {
  for (int i=0; i < strlen(to_check); i++)
    if(to_check[i] < 48 || to_check[i] > 57)
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
  if (argc!=3) exit_error(1, "Bad number of arguments (should be 2)");

  DIR *dir;
  struct dirent *dp;
  struct stat fileStat;
  char * path = argv[1];
  char * perm = argv[2];

  if (strlen(perm)!=3 || !check_char_int(perm)) exit_error(1, "Bad permissions");

  if ((dir = opendir(path)) == NULL) exit_error(1,"Cannot open dir");

  printf("File name:    \tFile size:       \tTime:      \n");
  while ((dp = readdir(dir)) != NULL) {
    char * tmp;
    if (path[strlen(path)-1]!='/') tmp = strcat(strcat(path,"/"), dp->d_name);
    else tmp = strcat(path,dp->d_name);

    if (stat(tmp,&fileStat) <1) continue;
    free(tmp);

    if(!S_ISREG(fileStat.st_mode)) continue;

    printf("%s\t%dB\t%ld\n", dp->d_name, (int) fileStat.st_size, fileStat.st_mtime);
  }

  closedir(dir);
  return 0;
}
