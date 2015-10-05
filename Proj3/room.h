#ifndef ROOM_H
#define ROOM_H
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <semaphore.h>
#include "rat.h"

using namespace std;

class Rat;

class Room
{
private:
    int roomCapacity; //Capacity of the room.
    int traversalTime; //Time a rat needs to sleep in the room.
    int roomID; //The ID of the room.
public:
    Room(int id, int capacity, int time);
    sem_t door;
    void EnterRoom();
    void LeaveRoom();
    int TryToEnterRoom();
    int getID();
    int getTraversalTime();
    int getCapacity();
    int getCost();
};

#endif
