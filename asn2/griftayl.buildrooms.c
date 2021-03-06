/*********************************************************************
CS 344 Program 2
griftayl.buildrooms.c
Taylor Griffin
02/08/18
*********************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Define roomName structure
struct roomName{
  char *names[10];
  int *taken[10];
};

// Define room structure
struct room {
  char* roomType;
  char* name;
  int numConnections;
  char* connections[6];
};

// Define roomData structure
struct roomData {
  struct room r[7];
};

createDirectory() {
  // Find process id and convert it to string
  int pid = getpid();
  char pidString[10];
  sprintf(pidString, "%d", pid);
  // Define first part of filename
  char user[20] = "./griftayl.rooms.";
  // Initilize filename
  char filename[100];
  // Add first part and pid to filename
  strcpy(filename, user);
  strcpy(filename+strlen(user), pidString);
  // Create directory
  mkdir(filename, 0700);
}

struct roomName createRoomName() {
  // Randomize
  srand(time(NULL));
  // List name options
  char* n[10] =    {"DOZER",
                    "TIGER",
                    "LAPIS",
                    "NAPIS",
                    "HENRY",
                    "LOGAN",
                    "BELLE",
                    "FLUFF",
                    "KITTY",
                    "TAFFE"
 };
 // Initlize roomNames var
  struct roomName roomNames;
  // Loop through to assign names and initlize taken
  int i=0;
  for (i=0;i<10;i++) {
      roomNames.names[i] = n[i];
      roomNames.taken[i] = 0;
    }
  // Return name data
  return(roomNames);
}

int connected(struct room r1,struct room r2) {
  int i;
  int ret = 0;
  for (i=0;i<r1.numConnections;i++) {
    if (r1.connections[i] == r2.name)
      ret = 1;
  }
  return(ret);
}

struct roomData* generateRoomData() {
  // Initlize rooms
  struct roomData* rooms = malloc(sizeof(struct roomData));
  // Initilize roomNames
  struct roomName roomNames = createRoomName();
  // Assign Room Names
  int i;
  for (i=0;i<7;i++) {
    int cont=0;
    // Loop until unused name is found
    while (cont == 0) {
      // Generate number between one and ten
      int r = rand() % 10;
      // Check whether index is taken
      cont = roomNames.taken[r];
      if (cont == 0) {
        // Break loop if name isn't taken
        cont = 1;
        // Set name
        rooms->r[i].name = roomNames.names[r];
        // Set name as taken
        roomNames.taken[r] = 1;
      }
      // Continue loop if name is taken
      else {
        cont = 0;
      }
    }
  }
  // Set room types
  for (i=0;i<7;i++) {
    if (i == 0)
      rooms->r[i].roomType = "START_ROOM";
    else if (i == 6)
      rooms->r[i].roomType = "END_ROOM";
    else
      rooms->r[i].roomType = "MID_ROOM";
  }
  // Set room connections
  for (i=0;i<7;i++) {
    int r = i;
    // Assign connections until room has three connections
    while (rooms->r[r].numConnections < 3) {
      int x = rand() % 7;
      // Check that randomly generated room is compatible to connect
      if ((x != r) && (rooms->r[x].numConnections < 6) && (connected(rooms->r[r],rooms->r[x]) == 0)) {
        // Add mutual connection
        rooms->r[r].connections[rooms->r[r].numConnections] = rooms->r[x].name;
        rooms->r[r].numConnections++;
        rooms->r[x].connections[rooms->r[x].numConnections] = rooms->r[r].name;
        rooms->r[x].numConnections++;
      }
    }
  }
  // Return generated room data
  return(rooms);
}

createRoomFiles(struct roomData* rooms) {
  // Initlize constant part of filepath
  int pid = getpid();
  char pidString[20];
  sprintf(pidString, "%d", pid);
  char user[20] = "./griftayl.rooms.";
  char filepath[100];
  strcpy(filepath, user);
  strcpy(filepath+strlen(user), pidString);
  strcpy(filepath+strlen(user)+strlen(pidString), "/room");
  // Loop through to create 7 and write files
  int i;
  char rm[6];
  char numChar[1];
  for (i=0;i<7;i++) {
    // Modify filepath
    sprintf(numChar,"%d", i);
    strcpy(filepath+strlen(user)+strlen(pidString)+5,numChar);
    // Open file for writing
    FILE* myFile = fopen(filepath, "w");
    // Write room name
    fprintf(myFile,"ROOM NAME: %s\n",rooms->r[i].name);
    int j;
    // Write connections
    for (j=0;j<rooms->r[i].numConnections;j++) {
      fprintf(myFile,"CONNECTION %d: %s\n",j+1,rooms->r[i].connections[j]);
    }
    // Write room type
    fprintf(myFile,"ROOM TYPE: %s\n",rooms->r[i].roomType);
    // Close file
    fclose(myFile);
  }
}

int main() {
  createDirectory();
  struct roomData* rooms = generateRoomData();
  createRoomFiles(rooms);
  free(rooms);
  return 0;
}
                                                                                                                       
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
