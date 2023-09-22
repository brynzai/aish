CC = g++
CFLAGS = -std=c++11 -Wall -O3 -I/usr/include/speech_tools -DAUDIO_MODE 
LIBS = -ljsoncpp -lcurl -lestools -lestbase -leststring -lasound -lncurses /usr/lib64/libFestival.a
SRC := $(wildcard *.cpp plugins/*.cpp) 
HDR := $(wildcard *.h)
OBJ := $(SRC:.cpp=.o)

all: aish

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

aish: $(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $(LIBS)

debug: $(SRC) $(HDR)
	$(CC) -o $@ $(CFLAGS) -g -DDEBUG $(SRC) $(LIBS)

clean:
	rm -f $(OBJ) aish debug static main
