CC = g++
CFLAGS = -Wall
MAINSRCS = main.cpp
MAINOUT = main.o
OUTPUTFILE = carrom

UNAME = `uname`
DetectOS:
	@echo "Building the source code"
	-@make $(UNAME)
	@echo "Done building the source code"
	@echo "Now, to run the game, simply type 'make run' or ./$(OUTPUTFILE)"

LDFLAGS+=-framework CoreFoundation -framework AudioToolbox

AUDIOSRCS = AudioPlayer.cpp

AUDIOOUT = AudioPlayer.o

ifeq ($(shell uname),Darwin)
	LIBS = -framework OpenGL -framework GLUT
	RMFILES = $(AUDIOOUT) $(MAINOUT) $(OUTPUTFILE)
else
	LIBS = -lglut -lGL
	RMFILES = $(OUTPUTFILE)
endif

Darwin:
	$(CC) -c -o $(MAINOUT) $(MAINSRCS)
	$(CC) -c -o $(AUDIOOUT) $(AUDIOSRCS)
	$(CC) $(CFLAGS) $(LIBS) $(LDFLAGS) $(MAINOUT) $(AUDIOOUT) -o $(OUTPUTFILE)

Linux:
	$(CC) $(MAINSRCS) $(CFLAGS) $(LIBS) -I"irrKlang/include" -L"/usr/lib" irrKlang/bin/linux-gcc-64/libIrrKlang.so -pthread -o $(OUTPUTFILE)

run:
	@echo "Running the carrom game"
	./$(OUTPUTFILE)

clean:
	@echo "Removing the build files"
	rm $(RMFILES)
