#include "room.h"


/**
 * Wait on the semaphore to enter the room.
 */
void Room::EnterRoom()
{
    sem_wait(&door);
}

/**
 * Signal that the rat is leaving the room through the
 * door semaphore.
 */
void Room::LeaveRoom()
{
    sem_post(&door);
}

/**
 * Try to enter the room using the sem_trywait 
 * method. This is used in the non-blocking 
 * traversal algorithm.
 * @return an int, the status of the wait request.
 */
int Room::TryToEnterRoom()
{
    return sem_trywait(&door);
}

/**
 * Get the room "cost". This cost is computed by multiplying 
 * the number of slots that are currently taken in the room 
 * by the traversal time of the room. This cost is used in 
 * determining which room to travel to next in the non-blocking
 * algorithm.
 * @return an int, the room cost.
 */
int Room::getCost()
{
    int emptySlots;
    sem_getvalue(&door, &emptySlots);
    emptySlots = (emptySlots < 0) ? 0 : emptySlots;
    int taken = roomCapacity - emptySlots;
    return taken * traversalTime;
}

