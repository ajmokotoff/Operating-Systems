#ifndef ROOM_H
#define ROOM_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <semaphore.h>
#include "rat.h"

using namespace std;

class Rat;

struct Room {
    int roomCapacity, traversalTime, roomID;
    sem_t door;

    Room(int id, int capacity, int time) : roomCapacity(capacity), traversalTime(time), roomID(id) {
        sem_init(&door, 0, roomCapacity);
    };

    void EnterRoom();
    void LeaveRoom();
    int TryToEnterRoom();
    int getCost();
};

#endif

