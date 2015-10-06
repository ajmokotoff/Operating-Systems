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
class Rat
{
private:
    /** Thread that defines the rat operations. */
    pthread_t _thread;
    int id; //Rat ID, passed in at object creation
    Maze* maze; //Pointer to the maze that the rat operates in
    int startingRoom; //The room that the rat is to start in
    int timeToComplete; //Accumulated time to maze completion
    int traversalMode; //0 is normal, 1 is non-blocking
    /**
     * Inline function needed to start the thread. This is needed
     * because the compiler is picky about what it will allow
     * to be started in the call to start the thread. Effectively
     * what this does is to re-define the function that performs
     * the maze traversal as static so that the compiler is happy.
     */
    static void * StartThreadFunction(void * rat)
    {
        return ((Rat *)rat)->Traverse(rat);
    };
public:
    Rat(int ratID, Maze* m, int room, int mode);
    virtual ~Rat() {};
    bool StartThread();
    bool JoinThread();
    int getTime();
    void * Traverse(void * rat);
};
#endif

