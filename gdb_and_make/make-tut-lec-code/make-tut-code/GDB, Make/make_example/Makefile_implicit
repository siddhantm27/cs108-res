CXX := g++
CFLAGS := -Wall -Wextra -std=c++11
# $< stands for the first prerequisite
main: main.o file.o directory.o
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

# main.o: main.cpp file.h directory.h
# 	$(CXX) $(CFLAGS) -c $< -o $@

# file.o: file.cpp file.h
# 	$(CXX) $(CFLAGS) -c $< -o $@

# directory.o: directory.cpp directory.h
# 	$(CXX) $(CFLAGS) -c $< -o $@

give_error:
	rmdir check
	echo hello

suppress_error: 
	-rmdir check
	echo hello

.PHONY: clean
clean:
	rm -f *.o main
