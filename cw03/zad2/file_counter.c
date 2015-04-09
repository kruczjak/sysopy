
void list_dir(char * path, int wait) {
  DIR *dir;
  struct dirent *dp;
  struct stat fileStat;

  if ((dir = opendir(path)) == NULL) return;

  while ((dp = readdir(dir)) != NULL) {
    char * tmp;
    if (path[strlen(path)-1]!='/') tmp = concat(path,"/", dp->d_name);
    else tmp = concat(path,"", dp->d_name);

    if (lstat(tmp,&fileStat) == -1) {
      free(tmp);
      continue;
    }

    if(S_ISDIR(fileStat.st_mode) && strcmp(dp->d_name,".")!=0 && strcmp(dp->d_name,"..")!=0) {

    }
    free(tmp);
  }

  closedir(dir);
}

int main(int argc, char ** argv) {
  if (argc == 2) {
    list_dir(argv[1], 0);
  } else if (argc == 3 && argv[1][0] == '-' && argv[1][1] == 'w') {
    list_dir(argv[2], 1);
  } else {
    printf("Bad arguments: file_counter [-w] <folder_path>");
  }
  return 0;
}
