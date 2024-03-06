all: udp-test

udp-test: udp-test.o sockutil.o timeutil.o
	cc -o udp-test udp-test.o sockutil.o timeutil.o -lm

udp-test.o: udp-test.c sockutil.h timeutil.h sync.h
	cc -c udp-test.c

sockutil.o: sockutil.c sockutil.h timeutil.h
	cc -c sockutil.c

timeutil.o: timeutil.c timeutil.h
	cc -c timeutil.c

clean:
	rm -f *.o udp-test

