CFLAGS = -g -Wall

ults: threadminator/threadminator.c ults.o $@ $^

.PHONY: clean
clean:
	rm -f *.o ults
