#include "rat.h"

/**
 * Start the rat thread, beginning the traversal of the maze.
 * @return true if traversal successful.
 */
bool Rat::StartThread()
{
    return (pthread_create(&_thread, NULL, &this->StartThreadFunction, this) == 0);
}

/**
 * Traverse the maze.
 * @param rat The rat that is traversing the maze. This is needed because
 * in the context of the thread, the thread has no idea which rat
 * <code>this</code> would refer to, therefore a reference to the 
 * calling rat needs to be passed in.
 */
void* Rat::Traverse(void* rat)
{
    if (!traversalMode)
    {
        int idx = ((Rat *)rat)->startingRoom;
        int visited = 0;
        Room * r;
        while ((unsigned long)visited < maze->rooms.size())
        {
            r = &maze->rooms.at(idx);
            r->EnterRoom();
            int entryTime = (int)difftime(time(NULL), maze->mazeStartTime);
            sleep(r->traversalTime);
            r->LeaveRoom();
            int exitTime = (int)difftime(time(NULL), maze->mazeStartTime);
            addToLogbook(idx, ((Rat *)rat)->id, entryTime, exitTime);
            visited++;
            idx++;
            if ((unsigned long)idx > maze->rooms.size() - 1)
                idx = 0;
        }
    } else
    {
        /**
         * The non blocking method follows this pathway:
         *
         * 10 Start in the room defined at rat creation.
         * 20 If this room has been visited, GOTO 40
         * 30 If the semaphore can be locked, enter the room and sleep
         * 40 If it can't, go to the "cheapest" room
         * 50 GOTO 20
         *
         * The definition of which room is cheapest is given in 
         * the <code>room</code> class.
         */
        int idx = ((Rat*)rat)->startingRoom;
        int visited = 0;
        int visitedRooms[MAXROOMS] = {};
        Room * r;
        while ((unsigned long) visited < maze->rooms.size())
        {
            if (visitedRooms[idx])
            {
                idx = getCheapestRoom(visitedRooms);
            }
            else
            {
                r = &maze->rooms.at(idx);
                if (!r->TryToEnterRoom())
                {
                    int entryTime = (int)difftime(time(NULL), maze->mazeStartTime);
                    sleep(r->traversalTime);
                    r->LeaveRoom();
            	    int exitTime = (int)difftime(time(NULL), maze->mazeStartTime);
                    addToLogbook(idx, ((Rat *)rat)->id, entryTime, exitTime);
                    visited++;
                    visitedRooms[idx] = 1;
                } else
                {
                    idx = getCheapestRoom(visitedRooms);
                }
            }
        }
    }
    ((Rat *)rat)->timeToComplete = (int)difftime(time(NULL), maze->mazeStartTime);;
    return NULL;
}

/**
 * Join the thread back upon completion.
 * @return True if joined successfully.
 */
bool Rat::JoinThread()
{   
    return (pthread_join(_thread, NULL) == 0);
}

