#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>
#include <string.h>

char * concat(char * first, char * middle, char * last) {
  char * ret = (char *) malloc(1+strlen(first)+strlen(middle)+strlen(last));
  strcpy(ret, first);
  strcat(ret, middle);
  strcat(ret, last);
  return ret;
}

int main(int argc, char **argv) {
  if (argc!=3) {
    perror("Bad number of arguments (should be 2)");
    exit(1);
  }
  DIR *dir;
  struct dirent *dp;
  struct stat fileStat;
  char * path = argv[1];
  char * perm = argv[2];
  if (strlen(perm)!=3)

  if ((dir = opendir(path)) == NULL) {
    perror("Cannot open dir");
    exit(1);
  }

  printf("File name:    \tFile size:       \tTime:      \n");
  while ((dp = readdir(dir)) != NULL) {
    char * tmp;
    if (path[strlen(path)-1]!='/') tmp = concat(path,"/",dp->d_name);
    else tmp = concat(path, "", dp->d_name);

    stat(tmp,&fileStat);
    free(tmp);

    if(!S_ISREG(fileStat.st_mode)) continue;

    printf("%s\t%dB\t%ld\n", dp->d_name, (int) fileStat.st_size, fileStat.st_mtime);
  }

  closedir(dir);
}
