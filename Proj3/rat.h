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
    pthread_t thread;
    int id, start_room, time_complete, traversal; // rat id, room to start in, time it took to finish, non blocking or blocking
    Maze* maze; // pointer to maze
    
    // constructor to create rat
    Rat(int ratID, Maze* m, int room, int mode) : id(ratID), maze(m), start_room(room), traversal(mode) {}

    bool StartThread(); // start rat thread
    bool JoinThread(); // end rat thread

    void* Traverse(void* rat); // traverse through maze
 
    //made a global static helper function to eliminate any possible bugs 
    static void* StartThreadHelper(void* rat) {
        return ((Rat *)rat)->Traverse(rat);
    };
};

#endif

