CC = g++
CFLAGS = -std=c++11 -Wall -O3
LIBS = -ljsoncpp -lcurl 
SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)

all: aish

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

aish: $(OBJ)
	$(CC) -o $@ $(CFLAGS) $(LIBS) $(OBJ)

debug: $(SRC)
	$(CC) -o $@ $(CFLAGS) -g -DDEBUG $(LIBS) $(SRC)

clean:
	rm -f *.o aish debug static main
