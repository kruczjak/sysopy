#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char ** argv) {
  pid_t child, child2;
  int ls_to_grep[2];
  int grep_to_wc[2];

  /* ls_to_grep */
  if (pipe(ls_to_grep)==-1) {
    perror("Failed to setup pipeline");
    return 1;
  }
  child = fork();

  if (child==0) {
    if (dup2(ls_to_grep[1], 1) == -1)
      perror("Failed to redirect stdout of ls");
    else if (close(ls_to_grep[0]) == -1)
      perror("Failed to close extra pipes");
    else {
      execl("/bin/ls", "ls", "-l", NULL);
      perror("Failed to execl ls");
    }
    return 1;
  }

  /* grep_to_wc */
  if (pipe(grep_to_wc)==-1) {
    perror("Failed to setup pipeline");
    return 1;
  }
  child2 = fork();

  if (child2==0) {
    if (dup2(ls_to_grep[0], 0) == -1)
      perror("Failed to redirect stdin to grep");
    else if (dup2(grep_to_wc[1], 1) == -1)
      perror("Failed to redirect stdout of grep");
    else if (close(ls_to_grep[1]) == -1 || close(grep_to_wc[0])== -1)
      perror("Failed to close extra pipes");
    else {
      execl("/bin/grep", "grep", "^d", NULL);
      perror("Failed to execl grep");
    }
    return 1;
  }

  close(ls_to_grep[0]); close(ls_to_grep[1]);

  if (dup2(grep_to_wc[0], 0) == -1)          /* sort jest rodzicem*/
    perror("Failed to redirect stdin of wc");
  else if (close(grep_to_wc[1]) == -1)
    perror("Failed to close extra pipe file descriptors on wc");
  else {
   execl("/bin/wc", "wc", "-l", NULL);
   perror("Failed to exec wc");

  }
  return 1;
}
