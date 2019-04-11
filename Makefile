CC=gcc
CXX=g++
RM=rm -f


CPPFLAGS=-std=c++11 -O3  -I.
LDFLAGS=-std=c++11 -O3  -pthread -lboost_system
LDLIBS=

PROGRAM=blockfinder_simple_int

SRCS=ncs.cpp \
     nmr.cpp \
     blockfinder.cpp \
     scheme.cpp \
     blockfinder_main.cpp \
     PatternCodes.cpp \
     tasks.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

all: $(PROGRAM)

ncs.o: ncs.cpp ncs.h

nmr.o: nmr.cpp ncs.h 

blockfinder.o: blockfinder.cpp blockfinder.h ncs.h

blockfinder_main.o: blockfinder_main.cpp blockfinder.h ncs.h

scheme.o: scheme.cpp scheme.h

PatternCodes.o: PatternCodes.cpp PatternCodes.h

tasks.o: tasks.cpp tasks.h

$(PROGRAM): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LDLIBS)

clean:
	rm -rf $(OBJS) $(PROGRAM)
