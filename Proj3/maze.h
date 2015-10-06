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

typedef struct {
    int iRat, tEntry, tDep;
} vbentry;

typedef struct {
    int max_rats, max_rooms; // max rats in maze, max rooms in maze
    deque<Room> rooms; //deque of all rooms
    deque<Rat> rats; //deque of all rats
    time_t maze_start; //time the maze started
    vbentry RoomVB[MAXROOMS][MAXRATS]; //array of room entries
    int VisitorCount[MAXROOMS]; //number of visitors array
    sem_t book_sem; // semaphore for book
} Maze;

void start_maze(string config_file, int maxRats, int maxRooms, char algorithm);
void run();
void addToLogbook(int room, int rat_ID, int time_entry, int time_exit);
int getCheapestRoom(int* visited);

#endif

