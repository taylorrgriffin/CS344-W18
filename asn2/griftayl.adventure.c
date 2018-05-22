/*********************************************************************
CS 344 Program 2
griftayl.adventure.c
Taylor Griffin
02/08/18
*********************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Initlize mutex
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

// Define room structure
struct room {
  char roomType[11];
  char name[6];
  int numConnections;
  char connections[6][6];
};

// Define roomData structure
struct roomData {
  struct room r[7];
};

int checkInput(char* input,struct room* roomCurr) {
  // Set up vars to check user input
  int i;
  int ret = -1;
  int num = roomCurr->numConnections;
  char roomCheck[6];
  char timeCheck[6] = "time\n";
  // Check if user input is an avaliable room
  for (i=0;i<num;i++) {
    while (strlen(roomCheck) > 0)
      roomCheck[strlen(roomCheck) - 1] = 0;
    strncpy(roomCheck,roomCurr->connections[i],5);
    roomCheck[5] = '\n';
    if (strcmp(input,roomCheck) == 0)
      ret = i;
  }
  // Check if user input is 'time'
  if (strcmp(input,timeCheck) == 0)
    ret = -2;
  return(ret);
}

struct room* changeRoom(int x,struct roomData* rooms,struct room* roomCurr) {
  int i;
  char* name = roomCurr->connections[x];
  // Loop through rooms to new one
  for (i=0;i<7;i++) {
    // Change roomCurr to new room
    if (strcmp(name,rooms->r[i].name) == 0) {
      roomCurr = &(rooms->r[i]);
      break;
    }
  }
  return(roomCurr);
}

void* writeTime(void* nothing) {
  // Lock mutex
  pthread_mutex_lock(&myMutex);
  // Delete file if it exists
  if(access("currentTime.txt",F_OK) != -1 ) {
    remove("currentTime.txt");
  }
  // Create file
  FILE* myFile = fopen("currentTime.txt", "w");
  // Set up vars for time
  time_t t;
  struct tm *tmp;
  char display[100];
  time(&t);
  // set time
  tmp = localtime( &t );
  strftime(display, sizeof(display), "%l:%M%P, %A, %B %d, 20%y\n", tmp);
  // print time to file
  fprintf(myFile,"%s",display);
  // Close file
  fclose(myFile);
  // Unlock mutex
  pthread_mutex_unlock(&myMutex);
}

void execute_game(struct roomData* rooms) {
  // Set up game
  int i;
  int win = 0;
  int check;
  int steps = 0;
  char* end = "END_ROOM";
  char* path [100];
  struct room* roomCurr;
  roomCurr = &(rooms->r[0]);
  // Set up user input
  int numCharsEntered = -5;
  int currChar = -5;
  size_t bufferSize = 0;
  char* lineEntered = NULL;
  // Lock mutex
  pthread_mutex_lock(&myMutex);
  // Set up time thread
  int timeThread;
  pthread_t thread;
  timeThread = pthread_create(&thread,NULL,writeTime,NULL);
  // Begin game
  while (win == 0) {
    // Print basic message
    printf("CURRENT LOCATION: %s\n",roomCurr->name);
    printf("POSSIBLE CONNECTIONS:");
    for (i=0;i<roomCurr->numConnections;i++) {
      printf(" %s",roomCurr->connections[i]);
      if ((roomCurr->numConnections - i) > 1)
        printf(",");
      else
        printf(".\n");
    }
    // Get user input
    printf("WHERE TO? >");
    numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
    // Print a blank line
    printf("\n");
    // Check user input
    check = checkInput(lineEntered,roomCurr);
    // Bad user input
    if (check == -1)
      printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    // User inputs time
    else if (check == -2) {
      // Enter time thread to print timestamp to file
      pthread_mutex_unlock(&myMutex);
      pthread_join(thread,NULL);
      pthread_mutex_lock(&myMutex);
      timeThread = pthread_create(&thread,NULL,writeTime,NULL);
      // Read timestamp from file, store and print timestamp
      char display[100];
      ssize_t nread;
      int fd = open("currentTime.txt", O_RDONLY);
      memset(display,'\0',sizeof(display));
      lseek(fd,0,SEEK_SET);
      nread = read(fd,display, sizeof(display));
      printf("%s\n",display);
    }
    // Good user input (room)
    else {
      // Change room, increment steps, and add to path
      roomCurr = changeRoom(check,rooms,roomCurr);
      path[steps] = roomCurr->name;
      steps++;
    }
    // Reset input stream
    free(lineEntered);
    lineEntered = NULL;
    // Check win condition
    if (strcmp(end,roomCurr->roomType) == 0)
      win = 1;
  }
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n",steps);
  for (i=0;i<steps;i++) {
    printf("%s\n",path[i]);
  }
  exit(0);
}

struct roomData* readRoomData(char* directory) {
  // Prepare vars to generate filenames
  char* n[7] = {"0","1","2","3","4","5","6"
  };
  int i;
  ssize_t nread;
  struct roomData* rooms = malloc(sizeof(struct roomData));
  char fullPath [60];
  char* pathMarker = "./";
  char* pathRoom = "/room";
  // Loop through all files
  for (i=0;i<7;i++) {
    // Write filenames
    strcpy(fullPath,pathMarker);
    strcpy(fullPath+2,directory);
    strcpy(fullPath+2+strlen(directory),pathRoom);
    strcpy(fullPath+2+strlen(directory)+5,n[i]);
    // Initlize buffer to read into
    char readBuffer[200];
    // Open file
    int fd = open(fullPath, O_RDONLY);
    // Rest file pointer position
    memset(readBuffer,'\0',sizeof(readBuffer));
    lseek(fd,0,SEEK_SET);
    // Read file into buffer
    nread = read(fd, readBuffer, sizeof(readBuffer));
    // Find the first colon and add two more bytes to get to the name
    int idx = 0;
    while (readBuffer[idx] != ':')
      idx++;
    idx+=2;
    // Set name & add null terminator
    strncpy(rooms->r[i].name,&readBuffer[idx],5);
    rooms->r[i].name[5] = '\0';
    // Start loop & move cursor to second line
    int done = 0;
    idx += 6;
    // Loop through connections
    rooms->r[i].numConnections = 0;
    while (done == 0) {
      int num = rooms->r[i].numConnections;
      // Set connections
      if (readBuffer[idx] == 'C') {
        idx += 14;
        strncpy(rooms->r[i].connections[num],&readBuffer[idx],5);
        rooms->r[i].connections[num][5] = '\0';
        rooms->r[i].numConnections++;
        // Move cursor to next line
        idx += 6;
      }
      // Set room type
      else {
        if (i == 0) {
          strcpy(rooms->r[i].roomType,"START_ROOM");
          rooms->r[i].roomType[10] = '\0';
        }
        else if (i == 6) {
          strcpy(rooms->r[i].roomType,"END_ROOM");
          rooms->r[i].roomType[8] = '\0';
        }
        else {
          strcpy(rooms->r[i].roomType,"MID_ROOM");
          rooms->r[i].roomType[8] = '\0';
        }
        // Break loop
        done = 1;
      }
    }
  }
  return(rooms);
}

char* determineDir() {
  // Initlize vars that will be used
  int i;
  int dirNum;
  time_t latest = 0;
  char latestDir [50];
  char* testString = "griftayl.rooms.";
  // Open directory
  DIR *dir = opendir(".");
  // Initlize directory pointer and stat
  struct dirent *selectedDir;
  struct stat sb;
  // Loop through all directories
  while ((selectedDir = readdir(dir)) != NULL) {
    // Select next directory to inspect
    stat(selectedDir->d_name, &sb);
     // Check that it's a directory of appropriate length
     if ((sb.st_mode == 16832) && (strlen(selectedDir->d_name) > 15)) {
       int safe = 1;
       for (i=0;i<15;i++) {
         // Check that it has the right first part of it's name
         if (selectedDir->d_name[i] != testString[i])
          safe = 0;
       }
       // Check time last modified
       if (safe == 1) {
         if (sb.st_mtime > latest) {
           latest = sb.st_mtime;
           strcpy(latestDir,selectedDir->d_name);
         }
       }
     }
  }
  // Close directory
  closedir(dir);
  // Return directory
  return(latestDir);
}

int main() {
  // Determine directory to read from
  char dir [50];
  strcpy(dir,determineDir());
  // Read in roomdata from selected directory
  struct roomData* rooms = readRoomData(dir);
  // Execute text-based game
  execute_game(rooms);
  // Free allocated memory
  free(rooms);
}
                              