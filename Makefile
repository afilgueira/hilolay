CFLAGS = -g -Wall

ults: hilolay/hilolay.c ults.o $@ $^

.PHONY: clean
clean:
	rm -f *.o ults
