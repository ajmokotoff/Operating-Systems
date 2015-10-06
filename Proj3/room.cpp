#include "room.h"


void Room::EnterRoom() {
    sem_wait(&door);
}


void Room::LeaveRoom() {
    sem_post(&door);
}


int Room::TryToEnterRoom() {
    return sem_trywait(&door);
}


int Room::getCost() {
    int emptySlots;
    
    if(sem_getvalue(&door, &emptySlots) < 0) {
        return roomCapacity * traversalTime;
    }
    return ((roomCapacity - emptySlots) * traversalTime);
}

