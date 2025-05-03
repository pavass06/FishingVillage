# Makefile for FishingVillage simulation

# Compiler
CC       := c++
OPT_FLAGS:= -O2

# Debug flags
DBG_FLAGS:= -Wall -Wextra -pedantic -Wshadow -Wconversion -Wnull-dereference 

# Remove some debug flags regarding unused variables
BG_FLAGS += -Wno-unused-parameter -Wno-unused-variable -Wno-unused-private-field

# Base compile flags
CFLAGS   := $(OPT_FLAGS) --std=c++17 -IAgent -IMarket -IWorld

# Link flags
LFLAGS   := -lstdc++

ifeq ($(DEBUG),1)
  CFLAGS += $(DBG_FLAGS) -Ddebug -g -Dverbose
endif

ifeq ($(VERBOSE),1)
  CFLAGS += -Dverbose
endif


# Where to put the runnable
RUNDIR   := ../wrk/
EXE      := agent.exe

# List exactly the .cpp files you have under src/
cppsource := \
    main.cpp \
    Agent/FisherMan.cpp \
    Market/JobMarket.cpp \
    Market/LabourModel.cpp

# Derive .o names
OBJCPP    := $(cppsource:.cpp=.o)
OBJ       := $(OBJCPP)

.PHONY: all prepare clean run flags objects

all: prepare $(EXE)

prepare:
	mkdir -p $(RUNDIR)

# Compile rule: .cpp → .o
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Link everything into the executable, then move it to RUNDIR
$(EXE): $(OBJ)
	$(CC) $(OPT_FLAGS) --std=c++17 $(OBJ) -o $@ $(LFLAGS)
	mv $@ $(RUNDIR)

run: all
	$(RUNDIR)$(EXE) ../data/input/test1.txt

flags:
	@echo "CFLAGS =" $(CFLAGS)
	@echo "LFLAGS =" $(LFLAGS)
	@echo "Sources:" $(cppsource)
	@echo "Objects:" $(OBJ)

objects:
	@echo $(OBJ)

clean:
	rm -f Agent/*.o Market/*.o *.o $(RUNDIR)$(EXE)
	rm -f $(RUNDIR)*.txt
