ifeq ($(shell uname), Darwin)
	APPLE_CCFLAGS = -m64
	APPLE_ASFLAGS = -arch x86_64
endif

CFLAGS = $(APPLE_CCFLAGS) -g -Wall

ults: ults.o ult_context_switch.o
	$(CC) $(APPLE_CCFLGS) -o $@ $^

.S.o:
	as $(APPLE_ASFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o ults
