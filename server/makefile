CC=g++
CFLAGS=-c -Wall -std=c++11
SOURCES=main.cpp tcpServer.cpp
OBJECTS=$(SOURCES:.cpp=.o)
LDFLAGS=-lpthread
EXECUTABLE=app.out

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) 

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o
