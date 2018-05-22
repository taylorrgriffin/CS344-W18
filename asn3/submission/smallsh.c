/*********************************************************************
CS 344 Program 3
smallsh.c
Taylor Griffin
02/26/18
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

#define BUFFER_SIZE 64
#define DELIM_CHAR " \t\r\n\a"

int backgProc [100];
char* backgArg0[100];
int stall = 1;
int toggleB = 0;
int backgFail;
int printKilled = 0;
int terminated = 0;
int backgroundEn = 1;
int lastStatus = -10000;
int exitStat = 0;
int firstJerk;

char* readLine() {
  // Get input from user
  char *line = NULL;
  ssize_t size = 0;
  getline(&line, &size, stdin);
  return line;
}

char** tokenize(char* line) {
  // Prepare tokens
  int bufferSize = BUFFER_SIZE;
  char **tokens = malloc(bufferSize * sizeof(char*));
  char *token;
  // Set initial position
  int pos = 0;
  // Store word in token
  token = strtok(line,DELIM_CHAR);
  // Store token in array
  while (token != NULL) {
    tokens[pos] = token;
    pos++;
    // Reallocate array if it's full
    if (pos>=bufferSize) {
      bufferSize+=BUFFER_SIZE;
      tokens = realloc(tokens, bufferSize * sizeof(char*));
    }
    // Reset token to empty
    token = strtok(NULL, DELIM_CHAR);
  }
  // Set last token to null
  tokens[pos] = NULL;
  // Return tokenized arguments
  return(tokens);
}

void setTerminated(int sig) {
  kill(getpid(),sig);
}

int execute(char** args) {
  backgFail = 0;
  stall = 0;
  // Catch blank line
  if  (args[0] == NULL) {
    return(1);
  }
  // Catch "exit"
  if (strcmp(args[0],"exit\0") == 0) {
    return(0);
  }
  // Catch "status"
  if (strcmp(args[0],"status\0") == 0) {
    // Check if it was terminated or not
    if (exitStat) {
      if (!lastStatus) {
        // successful (0) exit status
        printf("exit value %d\n",lastStatus);
      }
      else {
        // non-zero exit status
        printf("exit value 1\n");
      }
    }
    else {
      printf("terminated by signal %d\n",lastStatus);
    }
    return(1);
  }
  // Catch "cd"
  if (strcmp(args[0],"cd\0") == 0) {
    // No args
    if (args[1] == NULL) {
      char* path = getenv("HOME");
      if (chdir(path) != 0) {
        // nothing
      }
    }
    // Additional arg
    else {
      if (chdir(args[1]) != 0) {
        //nothing
      }
    }
    return(1);
  }
  // Catch comments
  if (args[0][0] == '#') {
    return(1);
  }
  // Check all args for var expansion
  int pos = 0;
  while (args[pos] != NULL) {
    if (strcmp(args[pos],"$$\0") == 0) {
      // Get PID and convert it to string
      int pid = getpid();
      char pidString[10];
      sprintf(pidString, "%d", pid);
      // Reset argumnent to NULL
      args[pos] = NULL;
      // Set argument to PID
      args[pos] = pidString;
    }
    pos++;
  }
  // Check whether it should be run in the background
  pos = 0;
  int background = 0;
  while (args[pos] != NULL) {
    if ((strcmp(args[pos],"&\0") == 0) && (args[pos+1] == NULL)) {
      // Set background var and remove & from args
      if (backgroundEn) {
        background = 1;
      }
      args[pos] = NULL;
    }
    pos++;
  }
  // Handle exec commands
  int check;
  int swap;
  // Create child process
  pid_t pid = fork();
  if ((background == 1) && (pid != 0)) {
    // Print out initiated background pid
    printf("background pid is %d\n",pid);
    int i;
    int done = 0;
    // find next avaliable index in array
    for (i=0;i<100;i++) {
      if ((backgProc[i] == -1) && (!done)) {
        // store pid at index
        backgProc[i] = pid;
        backgArg0[i] = args[0];
        done = 1;
      }
      if (done)
        break;
    }
  }
  // Have child process execute command
  if (pid == 0) {
    // Ignore Ctrl+Z in child process
    signal(SIGTSTP, NULL);
    // Catch Ctrl+C if foreground child process
    if (!background) {
      signal(SIGINT,setTerminated);
    }
    // Set default stdin and stdout for background processes
    if (background == 1) {
      int defIn = open("/dev/null", O_RDONLY);
      int defOut = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
      swap = dup2(defIn,0);
      swap = dup2(defOut,1);

    }
    // Check for stdin and stdout redirection
    firstJerk = -1;
    pos = 0;
    while (args[pos] != NULL) {
      // input redirection
      if (strcmp(args[pos],"<\0") == 0) {
        if (firstJerk == -1) {
          firstJerk = pos;
        }
        int newIn = open(args[pos+1], O_RDONLY);
        // Check if input file doesn't exist
        if (newIn == -1) {
          printf("cannot open %s for input\n",args[pos+1]);
          exit(1);
        }
        else {
          swap = dup2(newIn, 0);
        }
      }
      // output redirection
      if (strcmp(args[pos],">\0") == 0) {
        if (firstJerk == -1) {
          firstJerk = pos;
        }
        int newOut = open(args[pos+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        swap = dup2(newOut, 1);
      }
      pos++;
    }
    // Stop exec from seeing arguments past firstJerk
    if (firstJerk != -1) {
      args[firstJerk] = NULL;
    }
    if (execvp(*args, args) != 0) {
      if (!background) {
        perror(args[0]);
        exit(1);
      }
    }
    else {
      if (!background) {
        exit(0);
      }
    }
  }
  // Parent process
  else {
    // Check that it's a foreground proccess
    if (background == 0) {
      // Wait until child process is completed
      pid_t wait = waitpid(pid,&check,WUNTRACED);
      // Check if it was manually terminated
      if ((check == 2) || (check == 15)) {
        printf("terminated by signal %d\n",check);
        exitStat = 0;
      }
      else {
        exitStat = 1;
      }
      // Update for last foreground process status
      lastStatus = check;
    }
  }
  return(1);
}

void checkBackground() {
  int i;
  int pid;
  int check;
  for (i=0;i<100;i++) {
    // Found proccess
    if (backgProc[i] != -1) {
      // Set pid to to selected process pid
      pid = backgProc[i];
      // Check whether the process is completed
      pid_t chk = waitpid(pid,&check,WNOHANG);
      // Check process is done
      if (chk != 0) {
        // Check if process was manually terminated
        if ((check != 0) && (check != 1)) {
          printf("background pid %d is done: terminated by signal %d\n",pid,check);
        }
        else {
          printf("background pid %d is done: exit value %d\n",pid,check);
        }
        // Check if there's an error
        if (chk == 1) {
          perror(backgArg0[i]);
        }
        // Remove pid from temp array
        backgProc[i] = -1;
        backgArg0[i] = NULL;
      }
    }
  }
}

void killChildren() {
  int i;
  // For all processes
  for (i=0;i<100;i++) {
    if (backgProc[i] =! -1) {
      // Kill process
      kill(backgProc[i],2092);
      // Clear index
      backgProc[i] = -1;
    }
  }
}

void checkToggleBack() {
  // If background mode needs to be enabled
  if (toggleB) {
    // Disable &
    if (backgroundEn) {
      printf("Entering foreground-only mode (& is now ignored)\n");
      backgroundEn = 0;
    }
    // Enable &
    else {
      printf("Exiting foreground-only mode\n");
      backgroundEn = 1;
    }
    // Flip switch
    toggleB = 0;
  }
}

void loop() {
  // Initlialize vars needed
  char* line;
  char** args;
  int status = 1;
  // Loop while status isn't exit
  while (status) {
    // Check for background proccesses
    checkBackground();
    // Check for toggle background mode
    checkToggleBack();
    // Print leading character
    printf(": ");
    // Read user input, tokenize arguments, and run command
    line = readLine();
    args = tokenize(line);
    status = execute(args);
    // Flush text output
    fflush(stdout);
    // Free line and arguments
    free(line);
    free(args);
  }
  // Clean up background processes
  killChildren();
}

void toggleBackground() {
  // Flip switch to change background mode
  toggleB = 1;
}

int main() {
  // Catch Ctrl+Z in parent process
  signal(SIGTSTP, toggleBackground);
  // Ignore Ctrl+C in parent process
  struct sigaction sigIgnore;
  sigfillset(&sigIgnore.sa_mask);
  sigIgnore.sa_handler = SIG_IGN;
  sigIgnore.sa_flags = 0;
  sigaction(SIGINT, &sigIgnore, NULL);
  // Fill backgProc with all -1's
  int i;
  for (i=0;i<100;i++) {
    backgProc[i] = -1;
    backgArg0[i] = NULL;
  }
  // Execute main loop
  loop();
}
