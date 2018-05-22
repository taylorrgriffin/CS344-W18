/*********************************************************************
CS 344 Program 4
keygen.c
Taylor Griffin
03/12/18
*********************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void keygen(char* []);

int main(int argc,char* argv[]) {
  // There's enough arguments
  if (argc != 1) {
    keygen(argv);
  }
  else {
    printf("error: not enough args\n");
  }

}

void keygen(char** args) {
  int num = atoi(args[1]);
  char* key = malloc((num+1) * sizeof(char));
  // Generate number between 64 and 90
  int i;
  int r;
  char c;
  for (i=0;i<num;i++) {
    r = (rand() % 26) + 64;
    // insert spaces
    if (r == 64) {
      c = ' ';
    }
    else {
      c = r;
    }
    key[i] = c;
  }
  key[i] = '\n';
  printf("%s",key);
  free(key);
}
