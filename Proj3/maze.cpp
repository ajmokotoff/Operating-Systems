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
void start_maze(string configfile, int maxrats, int maxrooms, char alg)
{
    maze.maxRats = maxrats;
    maze.maxRooms = maxrooms;
    ifstream file;
    file.open(configfile.c_str(), ios::in);
    if (!file)
    {
        cerr << "File " << configfile << " could not be opened. Exiting..." << endl;
        exit(0);
    }
    cout << "Creating new maze with a maximum of " << maxrooms << " rooms and " << maxrats << " rats." << endl;
    cout << "Opened config file at " << configfile << endl;

    int capacity, delayTime, numrooms = 0;
    string str;
    while (getline(file, str) && (numrooms < maze.maxRooms))
    {
        //If there are more or less data points per line, bad file
        if (sscanf((char *)str.c_str(), "%i %i", &capacity, &delayTime) != 2)
        {
            cerr << "Bad description file!" << endl;
            exit(1);
        }
        Room r(numrooms, capacity, delayTime); //create and add room to list of rooms
        maze.rooms.push_back(r);
        numrooms++;
    }
    file.close();

    cout << "Spawning " << maze.maxRats << " rats..." << endl;
    int mode = (alg == 'n') ? 1 : 0;
    for (int i = 0; i < maze.maxRats; i++)
    {
        int rm;
        if (alg == 'i')
        {
            rm = 0;
        }
        else if ((unsigned long)maze.maxRats > maze.rooms.size())
        {
            rm = i % maze.rooms.size();
        }
        else
        {
            rm = i;
        }
        Rat r(i, &maze, rm, mode); //Create and add rats to list of rats.
        maze.rats.push_back(r);
    }
    sem_init(&maze.vbSem, 0, 1);
}

/**
 * Run the maze. This starts and kills all of the rats, and 
 * then prints out all of the results of the maze traversal.
 */
void run()
{
    cout << "\nLETTING LOOSE THE RATS...\n" << endl;
    maze.mazeStartTime = time(NULL);
    for (int i = 0; (unsigned long)i < maze.rats.size(); i++)
    {
        maze.rats.at(i).StartThread();
    }
    for (int i = 0; (unsigned long)i < maze.rats.size(); i++)
    {
        maze.rats.at(i).JoinThread();
    }
    int idealTime = 0;
    for (int i = 0; (unsigned long)i < maze.rooms.size(); i++)
    {
        cout << "Room " << i << " [" << maze.rooms.at(i).getTraversalTime() << " " << maze.rooms.at(i).getCapacity() << "]: ";
        idealTime += maze.rooms.at(i).getTraversalTime();
        for (int j = 0; (unsigned long)j < maze.rats.size(); j++)
        {
            cout << maze.RoomVB[i][j].iRat << " " << maze.RoomVB[i][j].tEntry << " " << maze.RoomVB[i][j].tDep << "; ";
        }
        cout << endl;
    }
    idealTime *= maze.rats.size();
    int totaltime = 0;
    for (int i = 0; (unsigned long)i < maze.rats.size(); i++)
    {
        cout << "Rat " << i << " completed maze in " << maze.rats.at(i).getTime() << " seconds." << endl;
        totaltime += maze.rats.at(i).getTime();
    }
    cout << "Total traversal time: " << totaltime << " seconds, compared to ideal time: " << idealTime << " seconds." << endl;
}

/**
 * Adds a data point to the specified logbook.
 * @param room The room to add an entry for.
 * @param ratID The rat to add an entry for.
 * @param timeEntry The time that the rat entered the room.
 * @param timeDep the time that the rat lef the room.
 */
void addToLogbook(int room, int ratID, int timeEntry, int timeDep)
{
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
int getCheapestRoom(int * visited)
{
    int room;
    int cost = 10000;
    for (unsigned long i = 0; i < maze.rooms.size(); i++)
    {
        if (visited[i] != 1)
        {
            int tmpcost = maze.rooms.at(i).getCost();
            if (tmpcost < cost)
            {
                room = i;
                cost = tmpcost;
            }
        }
    }
    return room;
}

int main(int argc, char** argv)
{
    //If input seems correct, proceed to running the maze
    if (argc == 3 && (argv[2][0] == 'i' || argv[2][0] == 'd' || argv[2][0] == 'n') && atoi(argv[1]) <= MAXRATS)
    {
        int rats = atoi(argv[1]);
        rats = (rats > MAXRATS) ? MAXRATS : rats;
        start_maze("./rooms", rats, MAXROOMS, argv[2][0]);
        cout<<"hello!"<<endl;
        run();
    }
    else //else exit and print usage.
    {
        cout << "Usage: ./maze [rats (0-" << MAXRATS << ")] [algorithm ([i]n order, [d]istributed, or [n]on-blocking)]" << endl;
        exit(0);
    }
}

