##
## Makefile for CS107 Assignment 2: Six Degrees
##

CXX = g++ -std=c++17
OPT=-O0 # no optimization of code
# generate files that encode make rules for the .h dependencies
DEPFLAGS=-MP -MD  # MD to generate dependency info, MP make each dependnecy file not dpend on anything to avoid errors
CPPFLAGS = -g -Wall $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)
LDFLAGS =

IMDB_CLASS = imdb.cc
IMDB_CLASS_H = $(IMDB_CLASS:.cc=.h)
IMDBTEST_SRCS = $(IMDB_CLASS) imdb-test.cc path.cc
IMDBTEST_OBJS = $(IMDBTEST_SRCS:.cc=.o)
IMDBTEST = imdb-test

MAINAPP_CLASS = $(IMDB_CLASS) path.cc
MAINAPP_CLASS_H = $(MAINAPP_CLASS:.cc=.h)
MAINAPP_SRCS = $(MAINAPP_CLASS) six-degrees.cc
MAINAPP_OBJS = $(MAINAPP_SRCS:.cc=.o)
MAINAPP = six-degrees

EXECUTABLES = $(IMDBTEST) $(MAINAPP)

TEST_SRC = unittests.cc
TESTOBJS = $(IMDB_CLASS:.cc=.o) $(TEST_SRC:.cc=.o) path.o
TESTBIN = $(TEST_SRC:.cc=.out)
BOOST_BIN = /usr/lib/x86_64-linux-gnu/libboost_unit_test_framework.a

DEPFILES = $(MAINAPP_SRCS:.cc=.d) $(TEST_SRC:.cc=.d)

default : $(EXECUTABLES)

$(IMDBTEST) : $(IMDBTEST_OBJS)
	$(CXX) -o $(IMDBTEST) $(IMDBTEST_OBJS) $(LDFLAGS)

$(IMDBTEST)-pure : $(IMDBTEST_OBJS)
	purify $(CXX) -o $(IMDBTEST).purify $(IMDBTEST_OBJS) $(LDFLAGS)

$(MAINAPP) : $(MAINAPP_OBJS)
	$(CXX) -o $(MAINAPP) $(MAINAPP_OBJS) $(LDFLAGS)

clean : 
	/bin/rm -f *.o a.out $(IMDBTEST) $(IMDBTEST).purify $(MAINAPP) $(MAINAPP).purify core Makefile.dependencies

immaculate: clean
	rm -fr *~

boosttest: $(TESTBIN)

$(TESTBIN): $(TESTOBJS)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -o $(TESTBIN) $(TESTOBJS) $(BOOST_BIN) 

%.o:%.cc
	$(CXX) $(CPPFLAGS) -c -o $@ $<

# NICE FLUFF
diff:
	$(info The status of the repository, and the volume of per-file changes:)
	@git status
	@git diff --stat

# include the dependencies
-include $(DEPFILES)
