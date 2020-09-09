CC=gcc
CXX=g++
RM=rm -f


HOST=$(shell hostname --short)
CPPFLAGS=-std=c++11 -O3 -msse3 -mavx -I.
LDFLAGS=-std=c++11 -O3  -pthread
LDLIBS=-lboost_thread -lboost_system -lboost_program_options -lboost_regex

BRANCH=more_valarray
PROGRAM=blockfinder_${BRANCH}_${HOST}
PROGRAM2=blockfinder

SRCS=ncs.cpp \
     nmr.cpp \
     blockfinder.cpp \
     scheme.cpp \
     blockfinder_main.cpp \
     PatternCodes.cpp \
     tasks.cpp

OBJS=$(subst .cpp,.o,$(SRCS))
ASMS=$(subst .cpp,.asm,$(SRCS))

all: $(PROGRAM)

ncs.o: ncs.cpp ncs.h

nmr.o: nmr.cpp ncs.h 

blockfinder.o: blockfinder.cpp blockfinder.h ncs.h

blockfinder_main.o: blockfinder_main.cpp blockfinder.h ncs.h

scheme.o: scheme.cpp scheme.h

PatternCodes.o: PatternCodes.cpp PatternCodes.h

tasks.o: tasks.cpp tasks.h

$(PROGRAM): $(OBJS)
	$(CXX) $(LDFLAGS) $(LDLIBS) -o $(PROGRAM) $(OBJS) $(LDLIBS)
	cp $(PROGRAM) $(PROGRAM2)

asm: $(ASMS)

%.asm: %.cpp
	$(CXX) $(LDFLAGS) $(LDLIBS) -S -o $@ $< $(LDLIBS)

pos_desc_example:
	g++ -std=c++11 -c pos_desc_example.cpp
	g++ -std=c++11 -lboost_program_options -o pos_desc_example pos_desc_example.o


clean:
	rm -rf $(OBJS) $(PROGRAM)
