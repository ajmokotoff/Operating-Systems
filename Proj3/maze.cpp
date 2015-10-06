#include "rat.h"
#include "maze.h"

Maze maze;

/* Create a maze which will encapsulate all the rooms and rats. It will open config_file to
 * determine the quantity.
 * @param config_file File to be read from
 * @param maxRats max number of rats in maze
 * @param maxRooms max number of rooms in maze
 * @param algorithm the algorithm to use
 */ 
void start_maze(string config_file, int maxRats, int maxRooms, char algorithm) {
    maze.max_rats = maxRats;
    maze.max_rooms = maxRooms;

    ifstream file;
    file.open(config_file, ios::in);
    if (!file) { // Make sure file isn't null
        cerr << "Error!: " << config_file << "." << endl;
        exit(0);
    }

    int capacity, delay_time, num_rooms = 0;
    string str;

    while (getline(file, str) && (num_rooms < maze.max_rooms)) { // scan line by line, looking for two integers and nothing more
        if (sscanf((char*)str.c_str(), "%i %i", &capacity, &delay_time) != 2) {
            cerr << "The config file is not formatted properly!" << endl;
            exit(1);
        }
        Room room(num_rooms, capacity, delay_time);
        maze.rooms.push_back(room);
        num_rooms++;
    }
    file.close();
    int mode = 0;
    if(algorithm == 'n') {
        mode = 1;
    }

    for (int i = 0; i < maze.max_rats; i++) {
        int r;
        if (algorithm == 'i') {
            r = 0;
        }
        else if (maze.max_rats > maze.rooms.size()) {
            r = i % maze.rooms.size();
        }
        else {
            r = i;
        }
        Rat rat(i, &maze, r, mode);
        maze.rats.push_back(rat);
    }
    sem_init(&maze.book_sem, 0, 1); // create a semaphore for the maze book so multiple threads do not write/read to it
}


/* Start the maze, create all the threads and semaphores, and run the maze.
 */
void run() {
    maze.maze_start = time(NULL); // intialize start time
    for (int i = 0; i < maze.rats.size(); i++) {
        maze.rats.at(i).StartThread(); // start a thread for each rat
    }
    for (int i = 0; i < maze.rats.size(); i++) {
        maze.rats.at(i).JoinThread(); // kill each rat thread
    }
    int ideal_time = 0;
    for (int i = 0; i < maze.rooms.size(); i++) { // run through each rat log to check for times
  	cout << "Room " << i << " [" << maze.rooms.at(i).traversal_time << " " << maze.rooms.at(i).room_capacity << "]: ";
        ideal_time += maze.rooms.at(i).traversal_time;
	for (int j = 0; j < maze.rats.size(); j++) {
            cout << maze.RoomVB[i][j].iRat << " " << maze.RoomVB[i][j].tEntry << " " << maze.RoomVB[i][j].tDep << "; ";
        }
	cout << endl;
    }
    ideal_time *= maze.rats.size();
    int total_time = 0;
    for (int i = 0; i < maze.rats.size(); i++) {
        cout << "Rat " << i << " completed maze in " << maze.rats.at(i).time_complete << " seconds." << endl;
	total_time += maze.rats.at(i).time_complete;
    }
    cout << "Total traversal time: " << total_time << " seconds, compared to ideal time: " << ideal_time << " seconds." << endl;
}

/* Add entry to the log book
 */

void addToLogbook(int room, int ratID, int timeEntry, int timeDep) {
    sem_wait(&maze.book_sem);
    maze.RoomVB[room][ratID].iRat = ratID;
    maze.RoomVB[room][ratID].tEntry = timeEntry;
    maze.RoomVB[room][ratID].tDep = timeDep;
    maze.VisitorCount[room] += 1;
    sem_post(&maze.book_sem);
}

/* Find room in the maze which has the least cost
 * @param visited array of roomIDs that the rat has seen
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


/* Main executable part of the program. Checks for input errors and then
 * creates a maze and starts the process.
 */
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

