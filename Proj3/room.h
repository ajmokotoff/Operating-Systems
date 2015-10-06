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
    int room_capacity, traversal_time, room_ID; // max capacity for a room, time of traversal, id for the room
    sem_t entry; // semaphore for each room

    Room(int id, int capacity, int time) : room_capacity(capacity), traversal_time(time), room_ID(id) {
        sem_init(&entry, 0, room_capacity);
    };

    void EnterRoom();
    void LeaveRoom();
    int TryToEnterRoom();
    int getCost();
};

#endif

