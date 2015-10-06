#ifndef RAT_H
#define RAT_H

#include <pthread.h>
#include <iostream>
#include "maze.h"
#include "room.h"
#include <unistd.h>

using namespace std;

/** need to define each of these classes so the compiler
 * doesn't screech at us
 */
class Room;

struct Rat {
    pthread_t _thread;
    int id; //Rat ID, passed in at object creation
    Maze* maze; //Pointer to the maze that the rat operates in
    int startingRoom; //The room that the rat is to start in
    int timeToComplete; //Accumulated time to maze completion
    int traversalMode; //0 is normal, 1 is non-bloc
    
    Rat(int ratID, Maze* m, int room, int mode) : id(ratID), maze(m), startingRoom(room), traversalMode(mode) {}

    bool StartThread();
    bool JoinThread();

    void* Traverse(void* rat);

    static void* StartThreadFunction(void* rat) {
        return ((Rat *)rat)->Traverse(rat);
    };
};

#endif

