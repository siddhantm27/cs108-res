MAIN_FOLDER := main/
SRC_FOLDER := src/
SRC_SUBFOLDER1 := $(SRC_FOLDER)src1/
SRC_SUBFOLDER2 := $(SRC_FOLDER)src2/
CXX := g++
CXX_FLAGS := -Wall --std=c++17

SRC1_FILES := $(wildcard $(SRC_SUBFOLDER1)*.cpp)
SRC2_FILEs := $(wildcard $(SRC_SUBFOLDER2)*.cpp)
SRC1_OBJS := $(SRC1_FILES:$(SRC_SUBFOLDER1)%.cpp=%.o)
SRC2_OBJS := $(SRC2_FILEs:$(SRC_SUBFOLDER2)%.cpp=%.o)
MAIN_FILES := $(wildcard $(MAIN_FOLDER)*.cpp)
EXECS := $(MAIN_FILES:$(MAIN_FOLDER)%.cpp=%)

all : $(EXECS) $(SRC1_OBJS) $(SRC2_OBJS)

main% : $(MAIN_FOLDER)main%.cpp $(SRC1_OBJS) $(SRC2_OBJS)
	$(CXX) $(CXX_FLAGS) -o $@ $^

%.o : $(SRC_SUBFOLDER1)%.cpp $(SRC_SUBFOLDER1)%.h
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

%.o : $(SRC_SUBFOLDER2)%.cpp $(SRC_SUBFOLDER2)%.h
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

clean :
	rm -f $(SRC1_OBJS) $(SRC2_OBJS) $(EXECS)
	rm -f $$(ls | grep -E main.+) *.o