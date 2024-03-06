files:= main.o file.o directory.o

main: $(files)
	g++ main.o file.o directory.o -o main

main.o: main.cpp file.h directory.h
	g++ -c main.cpp -o main.o

file.o: file.cpp file.h
	g++ -c file.cpp -o file.o

directory.o: directory.cpp directory.h
	g++ -c directory.cpp -o directory.o

.PHONY: clean
clean:
	rm *.o main