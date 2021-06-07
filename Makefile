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

TESTS = test_scheme-check

SRCS=ncs.cpp \
     nmr.cpp \
     blockfinder.cpp \
     scheme.cpp \
     PatternCodes.cpp \
     tasks.cpp \
     speedo.cpp \
     schemetest.cpp 

PROGRAM_MAIN=blockfinder_main.cpp 
TEST_SRCS=test_scheme-check.cpp

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c

OBJS=$(subst .cpp,.o,$(SRCS))
TEST_OBJS=$(subst .cpp,.o,$(TEST_SRCS))
PROG_MAIN=$(subst .cpp,.o,$(PROGRAM_MAIN))

all: $(PROGRAM) $(DEPDIR) $(TESTS)

%.o : %.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<


$(DEPDIR): 
	@mkdir -p $@

DEPFILES := $(SRCS:%.cpp=$(DEPDIR)/%.d)
$(DEPFILES):
include $(wildcard $(DEPFILES))



$(PROGRAM): $(OBJS) $(PROG_MAIN)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o $(PROGRAM) $(OBJS) $(PROG_MAIN) $(LDLIBS)
	cp $(PROGRAM) $(PROGRAM2)

test_scheme-check: $(OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $(OBJS) $(TEST_OBJS) $(LDLIBS)


pos_desc_example:
	g++ -std=c++11 -c pos_desc_example.cpp
	g++ -std=c++11 -lboost_program_options -o pos_desc_example pos_desc_example.o


clean:
	rm -rf $(OBJS) $(PROGRAM) $(DEPFILES)
