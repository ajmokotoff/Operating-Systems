#include "rat.h"


bool Rat::StartThread() {
    return (!(pthread_create(&_thread, NULL, &this->StartThreadFunction, this)));
}

bool Rat::JoinThread() {   
    return (!(pthread_join(_thread, NULL)));
}

void* Rat::Traverse(void* rat) {
    if (!traversalMode) {
        int idx = ((Rat *)rat)->startingRoom;
        int visited = 0;
        Room* r;
        while (visited < maze->rooms.size()) {
            r = &maze->rooms.at(idx);
            r->EnterRoom();
            int entryTime = (int)difftime(time(NULL), maze->mazeStartTime);
            sleep(r->traversalTime);

            r->LeaveRoom();
            int exitTime = (int)difftime(time(NULL), maze->mazeStartTime);
            addToLogbook(idx, ((Rat *)rat)->id, entryTime, exitTime);
            visited++;
            idx++;
            idx = (maze->rooms.size() - 1 < idx) ? 0:idx;
        }
    } else {
        int idx = ((Rat*)rat)->startingRoom;
        int visited = 0;
        int visitedRooms[MAXROOMS] = {};
        Room* r;
        while (visited < maze->rooms.size()) {
            if (visitedRooms[idx]) {
                idx = getCheapestRoom(visitedRooms);
            }
            else {
                r = &maze->rooms.at(idx);
                if (!(r->TryToEnterRoom())) {
                    int entryTime = (int)difftime(time(NULL), maze->mazeStartTime);
                    sleep(r->traversalTime);

                    r->LeaveRoom();
            	    int exitTime = (int)difftime(time(NULL), maze->mazeStartTime);
                    addToLogbook(idx, ((Rat *)rat)->id, entryTime, exitTime);
                    visited++;
                    visitedRooms[idx] = 1;
                } else {
                    idx = getCheapestRoom(visitedRooms);
                }
            }
        }
    }
    ((Rat *)rat)->timeToComplete = (int)difftime(time(NULL), maze->mazeStartTime);
    return NULL;
}

