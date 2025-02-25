# compilers
CC=c++

OPT_FLAGS= -O2
DBG_FLAGS=  -Wall -Wextra -pedantic -Wshadow  -Wconversion -Wnull-dereference

# compiling flags
CFLAGS= $(OPT_FLAGS) --std=c++17
# linking flags
LFLAGS += -lstdc++

ifeq ($(DEBUG),1)
 CFLAGS+= $(DBG_FLAGS) -Ddebug -g
endif 


# Include paths for headers
CFLAGS += -IAgent -IMarket -IWorld


LDIR =
RUNDIR = ../wrk/

EXE = agent.exe

cppsource+= main.cpp
# objects
OBJCPP = $(sort $(cppsource:.cpp=.o))


OBJ = $(OBJCPP)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDE_PATH)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS_PATH) $(LIBS)
	mv $(EXE) $(RUNDIR)

all: $(EXE)

prepare: 
	mkdir -p $(RUNDIR)

run: all
	mpirun --oversubscribe -np 2 $(RUNDIR)$(EXE)

.PHONY: clean all run

clean:
	rm -f *.o *~ core $(RUNDIR)$(EXE)
	rm -f $(RUNDIR)*.txt 

flags:
	@echo "RUNDIR=" $(RUNDIR) 
	@echo "CFLAGS=" $(CFLAGS) 
	@echo "LFLAGS=" $(LFLAGS) 
objects:
#	@echo "OBJ=" $(OBJ)
	@echo "files=" $(cppsource)
	

##########
define echored
    @tput setaf 1
    @echo $1
    @tput sgr0
endef
define echogreen
    @tput setaf 2
    @echo $1
    @tput sgr0
endef
define echoblue
    @tput setaf 4
    @echo $1
    @tput sgr0
endef
define echoinfo
    @tput setaf 2
    @echo $1 "  :  "  `tput setaf 0` $2
    @tput sgr0
endef


