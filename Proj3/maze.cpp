#include "rat.h"
#include "maze.h"

Maze maze;


void start_maze(string configfile, int maxrats, int maxrooms, char alg) {
    maze.maxRats = maxrats;
    maze.maxRooms = maxrooms;

    ifstream file;
    file.open(configfile, ios::in);
    if (!file) {
        cerr << "Error!: " << configfile << "." << endl;
        exit(0);
    }

    int capacity, delayTime, numrooms = 0;
    string str;

    while (getline(file, str) && (numrooms < maze.maxRooms)) {
        if (sscanf((char*)str.c_str(), "%i %i", &capacity, &delayTime) != 2) {
            cerr << "The config file is not formatted properly!" << endl;
            exit(1);
        }
        Room r(numrooms, capacity, delayTime);
        maze.rooms.push_back(r);
        numrooms++;
    }
    file.close();
    int mode;
    if(alg == 'n') {
        mode = 1;
    }
    else {
        mode = 0;
    }
    for (int i = 0; i < maze.maxRats; i++) {
        int rm;
        if (alg == 'i') {
            rm = 0;
        }
        else if (maze.maxRats > maze.rooms.size()) {
            rm = i % maze.rooms.size();
        }
        else {
            rm = i;
        }
        Rat r(i, &maze, rm, mode);
        maze.rats.push_back(r);
    }
    sem_init(&maze.vbSem, 0, 1);
}


void run() {
    maze.mazeStartTime = time(NULL);
    for (int i = 0; i < maze.rats.size(); i++) {
        maze.rats.at(i).StartThread();
    }
    for (int i = 0; i < maze.rats.size(); i++) {
        maze.rats.at(i).JoinThread();
    }
    int idealTime = 0;
    for (int i = 0; i < maze.rooms.size(); i++) { 
        cout << "Rat " << i << " completed maze in " << maze.rats.at(i).timeToComplete << " seconds." << endl;
        idealTime += maze.rooms.at(i).traversalTime;
    }
    idealTime *= maze.rats.size();
    int totaltime = 0;
    for (int i = 0; i < maze.rats.size(); i++) {
        totaltime += maze.rats.at(i).timeToComplete;
        cout << "Room " << i << " [" << maze.rooms.at(i).traversalTime << " " << maze.rooms.at(i).roomCapacity << "]: ";
        for (int j = 0; j < maze.rats.size(); j++) {
            cout << maze.RoomVB[i][j].iRat << " " << maze.RoomVB[i][j].tEntry << " " << maze.RoomVB[i][j].tDep << "; ";
        }
        cout << endl;
    }
    cout << "Total traversal time: " << totaltime << " seconds, compared to ideal time: " << idealTime << " seconds." << endl;
}

void addToLogbook(int room, int ratID, int timeEntry, int timeDep) {
    sem_wait(&maze.vbSem);
    maze.RoomVB[room][ratID].iRat = ratID;
    maze.RoomVB[room][ratID].tEntry = timeEntry;
    maze.RoomVB[room][ratID].tDep = timeDep;
    maze.VisitorCount[room] += 1;
    sem_post(&maze.vbSem);
}

int getCheapestRoom(int* visited) {
    int room;
    int cost = 10000;
    for (int i = 0; i < maze.rooms.size(); i++) {
        if (visited[i] != 1) {
            int tmpcost = maze.rooms.at(i).getCost();
            if (tmpcost < cost) {
                room = i;
                cost = tmpcost;
            }
        }
    }
    return room;
}

int main(int argc, char** argv) {
    //If input seems correct, proceed to running the maze
    if (argc != 3) {
        cout << "Error!" << endl;
        exit(0);
    }
    if(argv[2][0] != 'd' && argv[2][0] != 'n' && argv[2][0] != 'i') {
        cout << "Error!" << endl;
        exit(0);
    }
    if(atoi(argv[1]) > MAXRATS) {
        cout << "Error!" << endl;
        exit(0);
    }
    int rats = atoi(argv[1]);
    if(rats > MAXRATS) {
        rats = MAXRATS;
    }
    start_maze("./rooms", rats, MAXROOMS, argv[2][0]);
    run();
}

