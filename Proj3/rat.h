#ifndef RAT_H
#define RAT_H

#include <pthread.h>
#include <iostream>
#include "maze.h"
#include "room.h"
#include <unistd.h>

using namespace std;

class Room;

struct Rat {
    pthread_t _thread;
    int id, startingRoom, timeToComplete, traversalMode;
    Maze* maze;
    
    Rat(int ratID, Maze* m, int room, int mode) : id(ratID), maze(m), startingRoom(room), traversalMode(mode) {}

    bool StartThread();
    bool JoinThread();

    void* Traverse(void* rat);

    static void* StartThreadFunction(void* rat) {
        return ((Rat *)rat)->Traverse(rat);
    };
};

#endif

