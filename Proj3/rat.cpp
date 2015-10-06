#include "rat.h"

/* Start rat thread and return whether it worked
 */
bool Rat::StartThread() {
    return (!(pthread_create(&thread, NULL, &this->StartThreadHelper, this)));
}

/* End rat thread and return whether it worked
 */
bool Rat::JoinThread() {   
    return (!(pthread_join(thread, NULL)));
}

/* Function that lets the rat run through the maze
 */
void* Rat::Traverse(void* rat) {
    if (!traversal) { 
        int room_id = ((Rat *)rat)->start_room;
        int visited = 0;
        Room* room;
        while (visited < maze->rooms.size()) {
            room = &maze->rooms.at(room_id);
            room->EnterRoom();
            int entryTime = (int)difftime(time(NULL), maze->maze_start);
            sleep(room->traversal_time);

            room->LeaveRoom();
            int exitTime = (int)difftime(time(NULL), maze->maze_start);
            addToLogbook(room_id, ((Rat *)rat)->id, entryTime, exitTime);
            visited++;
            room_id++;
            room_id = (maze->rooms.size() - 1 < room_id) ? 0:room_id;
        }
    } else {
        int room_id = ((Rat*)rat)->start_room;
        int visited = 0;
        int visitedRooms[MAXROOMS] = {};
        Room* room;
        // non blocking algorithm begins below
        while (visited < maze->rooms.size()) {
            if (visitedRooms[room_id]) {
                room_id = getCheapestRoom(visitedRooms);
            }
            else {
                room = &maze->rooms.at(room_id);
                if (!(room->TryToEnterRoom())) {
                    int entry_time = (int)difftime(time(NULL), maze->maze_start);
                    sleep(room->traversal_time);

                    room->LeaveRoom();
            	    int exit_time = (int)difftime(time(NULL), maze->maze_start);
                    addToLogbook(room_id, ((Rat *)rat)->id, entry_time, exit_time);
                    visited++;
                    visitedRooms[room_id] = 1;
                } else {
                    room_id = getCheapestRoom(visitedRooms);
                }
            }
        }
    }
    ((Rat *)rat)->time_complete = (int)difftime(time(NULL), maze->maze_start);
    return NULL;
}

