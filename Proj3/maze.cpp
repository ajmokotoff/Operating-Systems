#include "rat.h"
#include "maze.h"

Maze maze;

/**
 * Constructs the maze object.
 * @param configfile The file to read in the room configuration from.
 * @param maxrats The maximum number of allowable rats in the maze.
 * @param maxrooms The maximum number of allowable rooms.
 * @param alg The algorithm to use (n, i, or d)
 */
void start_maze(string configfile, int maxrats, int maxrooms, char alg) {
    maze.maxRats = maxrats;
    maze.maxRooms = maxrooms;
    ifstream file;
    
    file.open(configfile, ios::in);
    if (!file) {
        cerr << "Error opening file: " << configfile << "." << endl;
        exit(0);
    }
    cout << "Initializing a new maze..." << endl;
    

    int capacity, delayTime, numrooms = 0;
    string str;

    while (getline(file, str) && (numrooms < maze.maxRooms)) {
        if (sscanf((char *)str.c_str(), "%i %i", &capacity, &delayTime) != 2) {
            cerr << "File does not contain data in the correct layout (integer integer)." << endl;
            exit(1);
        }
        Room r(numrooms, capacity, delayTime); //create and add room to list of rooms
        maze.rooms.push_back(r);
        numrooms++;
    }
    file.close();
    int mode;
    cout << "Creating " << maze.maxRats << " rats!" << endl;
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
        else if ((unsigned long)maze.maxRats > maze.rooms.size()) {
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

/**
 * Run the maze. This starts and kills all of the rats, and 
 * then prints out all of the results of the maze traversal.
 */
void run() {
    cout << "\nRats are entering the maze.\n" << endl;
    maze.mazeStartTime = time(NULL);
    for (int i = 0; i < maze.rats.size(); i++)
    {
        maze.rats.at(i).StartThread();
    }
    for (int i = 0; i < maze.rats.size(); i++)
    {
        maze.rats.at(i).JoinThread();
    }
    int idealTime = 0;
    for (int i = 0; i < maze.rooms.size(); i++)
    {
        cout << "Room " << i << " [" << maze.rooms.at(i).getTraversalTime() << " " << maze.rooms.at(i).getCapacity() << "]: ";
        idealTime += maze.rooms.at(i).getTraversalTime();
        for (int j = 0; j < maze.rats.size(); j++)
        {
            cout << maze.RoomVB[i][j].iRat << " " << maze.RoomVB[i][j].tEntry << " " << maze.RoomVB[i][j].tDep << "; ";
        }
        cout << endl;
    }
    idealTime *= maze.rats.size();
    int totaltime = 0;
    for (int i = 0; i < maze.rats.size(); i++)
    {
        cout << "Rat " << i << " completed maze in " << maze.rats.at(i).getTime() << " seconds." << endl;
        totaltime += maze.rats.at(i).getTime();
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

/**
 * Get the room in the maze with the "lowest cost". This is 
 * found by looping through each room and finding the one that
 * reports the lowest cost, and then returning that one's ID.
 * This also takes into account which rooms the rat has 
 * previously visited.
 * @param visited An array of integers representing room IDs
 * that the rat has already visited.
 * @return An int, the room ID of the cheapest room in the maze.
 */
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
        if(!(argv[2][0] == 'd' || argv[2][0] == 'n' || argv[2][0] == 'i')) {
            cout << "Error!" << endl;
            exit(0);
            if(atoi(argv[1]) > MAXRATS) {
                cout << "Error!" << endl;
                exit(0);
            }
        }
    }
    int rats = atoi(argv[1]);
    if(rats > MAXRATS) {
        rats = MAXRATS;
    }
    start_maze("./rooms", rats, MAXROOMS, argv[2][0]);
    run();
}

