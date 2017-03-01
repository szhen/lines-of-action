# Makefile
FLAGS = -std=c++11 -Wall -O1 -g -D _POSIX_C_SOURCE=200112L -D _GNU_SOURCE

all: loa

loa: linesofaction.cpp
	g++ ${FLAGS} -lstdc++ -o loa linesofaction.cpp

clean:
	rm -f loa