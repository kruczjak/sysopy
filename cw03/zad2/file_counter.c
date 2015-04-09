
void list_dir(char * path, int perm) {
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

    if(S_ISDIR(fileStat.st_mode) && strcmp(dp->d_name,".")!=0 && strcmp(dp->d_name,"..")!=0) list_dir(tmp, perm);
    free(tmp);
  }

  closedir(dir);
}

int main(int argc, char ** argv) {

  return 0;
}
