CPP=gcc -E
CC=gcc
CXX=g++
RM=rm -f


HOST=$(shell hostname --short)
BRANCH=$(shell git name-rev --name-only HEAD)
DEPDIR := .deps
CPPFLAGS= -I. 
CXXFLAGS= -std=c++11 -O3 
LDFLAGS= -pthread
LDLIBS=-lboost_thread -lboost_system -lboost_program_options -lboost_regex

PROGRAM=blockfinder_${BRANCH}_${HOST}
PROGRAM2=blockfinder

SRCS=ncs.cpp \
     nmr.cpp \
     blockfinder.cpp \
     scheme.cpp \
     blockfinder_main.cpp \
     PatternCodes.cpp \
     tasks.cpp \
     speedo.cpp

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c

OBJS=$(subst .cpp,.o,$(SRCS))

all: $(PROGRAM)

%.o : %.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

$(DEPDIR): ; @mkdir -p $@

DEPFILES := $(SRCS:%.cpp=$(DEPDIR)/%.d)
$(DEPFILES):
include $(wildcard $(DEPFILES))



counter.o: counter.cpp counter.h

$(PROGRAM): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o $(PROGRAM) $(OBJS) $(LDLIBS)
	cp $(PROGRAM) $(PROGRAM2)


pos_desc_example:
	g++ -std=c++11 -c pos_desc_example.cpp
	g++ -std=c++11 -lboost_program_options -o pos_desc_example pos_desc_example.o


clean:
	rm -rf $(OBJS) $(PROGRAM) $(DEPFILES)
