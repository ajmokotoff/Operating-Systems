#ifndef MAZE_H
#define MAZE_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "room.h"
#include <fstream>
#include <cstring>
#include "rat.h"
#include <time.h>
#include <deque>

#define MAXRATS 5
#define MAXROOMS 8

using namespace std;

class Room;
class Rat;

typedef struct{
    int iRat; //rat identifier
    int tEntry; //Time of rat entry
    int tDep; //time of rat departure
} vbentry;

typedef struct{
    int maxRats;
    int maxRooms;
    deque<Room> rooms;
    deque<Rat> rats;
    time_t mazeStartTime;
    vbentry RoomVB[MAXROOMS][MAXRATS];
    int VisitorCount[MAXROOMS];
    sem_t vbSem;
} Maze;

void start_maze(string configfile, int maxrats, int maxrooms, char alg);
void run();
void addToLogbook(int room, int ratID, int timeEntry, int timeDep);
int getCheapestRoom(int * visited);

#endif

