#include "rat.h"

/**
 * Create the rat object.
 * @param ratID The ID for the rat.
 * @param m The maze that the rat lives in.
 * @param mode The traversal mode, 0 if normal 1 if non-blocking
 */
Rat::Rat(int ratID, Maze * m, int room, int mode)
{
    id = ratID;
    maze = m;
    startingRoom = room;
    traversalMode = mode;
    cout << "Rat " << id << " created, starting in room " <<
        startingRoom << "." << endl;
}

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
void * Rat::Traverse(void * rat)
{
    if (traversalMode == 0)
    {
        int idx = ((Rat *)rat)->startingRoom;
        int visited = 0;
        Room * r;
        while ((unsigned long)visited < maze->rooms.size())
        {
            r = &maze->rooms.at(idx);
            r->EnterRoom();
            int entryTime = getTimeDiffSeconds();
            sleep(r->getTraversalTime());
            r->LeaveRoom();
            int exitTime = getTimeDiffSeconds();
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
        int idx = ((Rat *)rat)->startingRoom;
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
                if (r->TryToEnterRoom() == 0)
                {
                    int entryTime = getTimeDiffSeconds();
                    sleep(r->getTraversalTime());
                    r->LeaveRoom();
                    int exitTime = getTimeDiffSeconds();
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
    ((Rat *)rat)->timeToComplete = getTimeDiffSeconds();
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

/**
 * Get the time that the rat took to traverse
 * the maze.
 * @return int Traversal time in seconds.
 */
int Rat::getTime()
{
    return timeToComplete;
}
