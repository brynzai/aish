CC              = g++
CFLAGS          = -std=c++11 -Wall -I/usr/include/speech_tools ${ENV_CFLAGS}
LIBS            = -ljsoncpp -lcurl
LIBSAUDIO       = $(LIBS) -lestools -lestbase -leststring -lasound -lncurses /usr/lib64/libFestival.a
SRC             := $(wildcard *.cpp plugins/*.cpp)
HDR             := $(wildcard *.h)
OBJ             := $(SRC:.cpp=.o)

all: aish

.cpp.o:
        $(CC) $(CFLAGS) -c -o $@ $<

aish: $(OBJ)
        $(CC) -O3 -o $@ $(CFLAGS) $(OBJ) $(LIBSAUDIO)

aish_without_audio: $(OBJ)
        $(CC) -O3 -o aish $(CFLAGS) $(OBJ) $(LIBS)

debug: $(SRC) $(HDR)
        $(CC) -o $@ $(CFLAGS) -g -DDEBUG $(SRC) $(LIBS)

clean:
        rm -f $(OBJ) aish debug static main
