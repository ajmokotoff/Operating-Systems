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
    int roomCapacity; //Capacity of the room.
    int traversalTime; //Time a rat needs to sleep in the room.
    int roomID; //The ID of the room.
    sem_t door;

    Room(int id, int capacity, int time) : roomCapacity(capacity), traversalTime(time), roomID(id) {
        sem_init(&door, 0, roomCapacity);
        cout << "Created room " << id << " with capacity " << roomCapacity << " and traversal time of " << time << " seconds." << endl;
    };

    void EnterRoom();
    void LeaveRoom();
    int TryToEnterRoom();
    int getCost();
};

#endif
