TARGETS = visualizer
CC = g++
CFLAGS = `pkg-config opencv --cflags` -g -Wall -save-temps -O3 -std=c++0x
LIBS = `pkg-config opencv --libs`

all:$(TARGETS)

#visualizer.o: visualizer.hpp visualizer.cpp
#	$(CC) $(CFLAGS) -c visualizer.cpp

visualizer: main.cpp visualizer.cpp
	$(CC) $(CFLAGS) main.cpp visualizer.cpp -o visualizer $(LIBS) 
	rm -rf *.dSYM

clean:
	rm visualizer *.ii *.s *.o *.avi *.mp4
