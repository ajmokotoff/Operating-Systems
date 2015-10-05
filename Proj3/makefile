all: maze

maze: room.o rat.o maze.o
	g++ room.o rat.o maze.o -lpthread -o maze
	rm *.o

room.o: room.h room.cpp rat.h
	g++ -std=c++0x -c room.cpp

rat.o: rat.h rat.cpp room.h
	g++ -std=c++0x -c rat.cpp

maze.o: maze.h maze.cpp room.h
	g++ -std=c++0x -c maze.cpp

clean:
	rm maze

