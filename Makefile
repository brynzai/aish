CC		= g++
CFLAGS		+= -std=c++11 -Wall -O3 -I/usr/include/speech_tools
LIBS		= -ljsoncpp -lcurl
LIBSAUDIO	= $(LIBS) -lestools -lestbase -leststring -lasound -lncurses
SRC		:= $(wildcard *.cpp plugins/*.cpp)
HDR		:= $(wildcard *.h)
OBJ		:= $(SRC:.cpp=.o)

all: aish

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

aish: $(OBJ)
	$(CC) -DAUDIO_MODE -o $@ $(CFLAGS) $(OBJ) $(LIBSAUDIO)

aish_without_audio: $(OBJ)
	$(CC) -o aish $(CFLAGS) $(OBJ) $(LIBS)

debug: $(SRC) $(HDR)
	$(CC) -o $@ $(CFLAGS) -g -DDEBUG $(SRC) $(LIBS)

clean:
	rm -f $(OBJ) aish debug static main
