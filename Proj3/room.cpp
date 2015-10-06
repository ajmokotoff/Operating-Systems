#include "room.h"

// Enter room when semaphore is ready
void Room::EnterRoom() {
    sem_wait(&entry);
}

// Tell the door semaphore the rat is ready to leave
void Room::LeaveRoom() {
    sem_post(&entry);
}

// attempt to enter room using semaphore trywait method
int Room::TryToEnterRoom() {
    return sem_trywait(&entry);
}

//used for the non-blocking algorithm, it determines the room cost
int Room::getCost() {
    int empty_slots;
    
    if(sem_getvalue(&entry, &empty_slots) < 0) {
        return room_capacity * traversal_time;
    }
    return ((room_capacity - empty_slots) * traversal_time);
}

