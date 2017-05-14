###############################################################################
# compiler setting
###############################################################################
CC       = gcc
CXX      = g++
CFLAGS   = -g -Wall
CXXFLAGS = $(CFLAGS)
LIBS     = -lm -lpthread
INCPATH  =
DIR     = $(shell pwd)

###############################################################################
# source files setting
###############################################################################
CXX_SOURCES = $(shell find . -name "*.cpp")
#CXX_OBJS    = $(patsubst %.cpp,%.o,$(wildcard $(CXX_SOURCES)))

###############################################################################
.PHONY : clean clean_all
###############################################################################

OBJS        =
OBJS_CLIENT = client.o
OBJS_SERVER = server.o


EXEC_SERVER = server
EXEC_CLIENT = client
EXEC        = $(EXEC_SERVER) $(EXEC_CLIENT)

all: $(EXEC)

%.o:%.cpp hysocket.h hyremote_commands.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) $< -o $@

$(EXEC_SERVER): $(OBJS_SERVER)
	$(CXX) $(OBJS) $< -o $@ $(LIBS)

$(EXEC_CLIENT): $(OBJS_CLIENT)
	$(CXX) $(OBJS) $< -o $@ $(LIBS)
###############################################################################
clean:
	@rm -vfr *.o $(EXEC)
clean_all: clean
	@rm -vfr $(EXEC)
###############################################################################

