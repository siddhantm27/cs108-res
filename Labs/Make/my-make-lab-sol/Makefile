OBJ:= comp.o find.o ops.o add.o rem.o
F:=  -Wall --std=c++17
all : src main

src : src1 src2

src1 : comp.o find.o ops.o

comp.o : ./src/src1/comp.cpp ./src/src1/comp.h
	g++ $F -c -o comp.o -I ./src/src1 ./src/src1/comp.cpp 

find.o : ./src/src1/find.cpp ./src/src1/find.h
	g++ $F -c -o find.o -I ./src/src1 ./src/src1/find.cpp

ops.o : ./src/src1/ops.cpp ./src/src1/ops.h
	g++ $F -c -o ops.o -I ./src/src1 ./src/src1/ops.cpp

src2 : add.o rem.o

add.o : ./src/src2/add.cpp ./src/src2/add.h
	g++ $F -c -o add.o -I ./src/src2 ./src/src2/add.cpp

rem.o : ./src/src2/rem.cpp ./src/src2/rem.h
	g++ $F -c -o rem.o -I ./src/src2 ./src/src2/rem.cpp

main : main1 main2 main3 main4 main5 main6 main7 main8 main9

main1 : ./main/main1.cpp src1 src2
	g++ $F -o main1 ./main/main1.cpp $(OBJ)

main2 : ./main/main2.cpp src1 src2
	g++ $F -o main2 ./main/main2.cpp $(OBJ)

main3 : ./main/main3.cpp src1 src2
	g++ $F -o main3 ./main/main3.cpp $(OBJ)

main4 : ./main/main4.cpp src1 src2
	g++ $F -o main4 ./main/main4.cpp $(OBJ)

main5 : ./main/main5.cpp src1 src2
	g++ $F -o main5 ./main/main5.cpp $(OBJ)

main6 : ./main/main6.cpp src1 src2
	g++ $F -o main6 ./main/main6.cpp $(OBJ)

main7 : ./main/main7.cpp src1 src2
	g++ $F -o main7 ./main/main7.cpp $(OBJ)

main8 : ./main/main8.cpp src1 src2
	g++ $F -o main8 ./main/main8.cpp $(OBJ)

main9 : ./main/main9.cpp src1 src2
	g++ $F -o main9 ./main/main9.cpp $(OBJ)

clean: 
	rm *.o main[0-9]

