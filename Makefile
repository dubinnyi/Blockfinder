CC=gcc
CXX=g++
RM=rm -f

PROGRAMM=blockfinder_fast_parallel

CPPFLAGS=-O2 -std=c++11 -g -pg
LDFLAGS=-O2 -std=c++11 -g -pg
LDLIBS=

SRCS=ncs.cpp \
     classes.cpp \
     blockfinder.cpp \
     scheme.cpp \
     blockfinder_main.cpp \
     PatternCodes.cpp \
     tasks.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

all: $(PROGRAMM)

ncs.o: ncs.cpp ncs.h

classes.o: classes.cpp ncs.h

blockfinder.o: blockfinder.cpp blockfinder.h ncs.h

blockfinder_main.o: blockfinder_main.cpp blockfinder.h ncs.h

scheme.o: scheme.cpp scheme.h

PatternCodes.o: PatternCodes.cpp PatternCodes.h

tasks.o: tasks.cpp tasks.h

$(PROGRAMM): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(PROGRAMM) $(OBJS) $(LDLIBS)

#test:
#	./test_ncs

clean:
	$(RM) $(OBJS) $(PROGRAMM)

