#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

void info(int e) {
  printf("Options:\n");
  printf("-h\t--help\tShow this info\n");
	printf("-v\t--verbose\tExtra information\n");
	printf("-w\t--wait\tSleep for 15s\n");
  exit(e);
}
char * concat(char * first, char * middle, char * last) {
  char * ret = (char *) malloc(1+strlen(first)+strlen(middle)+strlen(last));
  strcpy(ret, first);
  strcat(ret, middle);
  strcat(ret, last);
  return ret;
}

int main(int argc, char ** argv) {
  static struct option options[] = {
    {"help", no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'v'},
		{"wait", no_argument, 0, 'w'},
		{0, 0, 0, 0}
  };

  bool verbose = false;
  bool waitP = false;
  int i=0;
  if (argc == 1) info(EXIT_FAILURE);
  while(true) {
    int opts = getopt_long(argc, argv, "hvw", options, &i);
    if (opts == -1) break;
    switch(opts)
		{
			case 'v': verbose = true;
						break;
			case 'h': info(EXIT_SUCCESS);
						break;
			case 'w': waitP = true;
						break;
			default: info(EXIT_FAILURE);
            break;
		}
  }

  char * path = argv[argc-1];
  if (path == NULL) info(EXIT_FAILURE);

  DIR *dir;
  struct dirent *dp;
  struct stat fileStat;
  int filesOverall = 0;
  int dirs = 0;

  pid_t PID;
  int childProcesses = 0;

  if ((dir = opendir(path)) == NULL) {
    fprintf(stderr, "Error while opening dir");
    exit(EXIT_FAILURE);
  }

  while ((dp = readdir(dir)) != NULL) {
    if (strcmp(dp->d_name,".")!=0 && strcmp(dp->d_name,"..")!=0) {
      char * tmp;
      if (path[strlen(path)-1]!='/') tmp = concat(path,"/", dp->d_name);
      else tmp = concat(path,"", dp->d_name);

      if (lstat(tmp,&fileStat) == -1) {
        free(tmp);
        continue;
      }

      if(S_ISDIR(fileStat.st_mode)) {

        childProcesses++;
        PID = fork();
        if (PID == -1) {
          childProcesses--;
          fprintf(stderr, "Fork error\n");
          return EXIT_FAILURE;
        } else if (PID == 0) {
          argv[argc-1] = tmp;
          if(execvp(argv[0], argv) < 0)
            fprintf(stderr, "Exec error\n");
        }
      } else if (S_ISREG(fileStat.st_mode))
        filesOverall++;

      free(tmp);
    }
  }

  closedir(dir);

  if (waitP) sleep(15);
  int s;
  while(childProcesses--) {
    wait(&s);
    dirs += WEXITSTATUS(s);
  }
  if (verbose) printf("Processing: %s \x1B[32mMy: %d \x1B[33mMy and children: %d\x1B[0m\n",
    path, filesOverall, filesOverall + dirs);
  _exit(dirs + filesOverall);
  return 0;
}
