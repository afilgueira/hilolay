#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	MaxUlts = 4,
	StackSize = 0x400000,
};

struct tcb {
	struct tcbContext {
		uint64_t rsp;
		uint64_t r15;
		uint64_t r14;
		uint64_t r13;
		uint64_t r12;
		uint64_t rbx;
		uint64_t rbp;
	} context;
	enum {
		Unused,
		Running,
		Ready,
	} state;
};

struct tcb ults[MaxUlts];
struct tcb *currentUlt;

void ult_init(void);
void ult_return(int ret);
void ult_context_switch(struct tcbContext *old, struct tcbContext *new);
bool ult_yield(void);
static void ult_stop(void);
int ult_start(void (*f)(void));

void ult_init(void) {
	currentUlt = &ults[0];
	currentUlt->state = Running;
}

void __attribute__((noreturn))ult_return(int ret) {
	if (currentUlt != &ults[0]) {
		currentUlt->state = Unused;
		ult_yield();
		assert(!"reachable");
	}
	while (ult_yield());
	exit(ret);
}

bool ult_yield(void) {
	struct tcb *p;
	struct tcbContext *old, *new;

	p = currentUlt;
	while (p->state != Ready) {
		if (++p == &ults[MaxUlts])
			p = &ults[0];
		if (p == currentUlt)
			return false;
	}

	if (currentUlt->state != Unused) {
		currentUlt->state = Ready;
	}
	p->state = Running;

	old = &currentUlt->context;
	new = &p->context;

	currentUlt = p;
	ult_context_switch(old, new);

	return true;
}

static void ult_stop(void) {
	ult_return(0);
}

int ult_start(void (*f)(void)) {
	char *stack;
	struct tcb *p;

	for (p = &ults[0];; p++) {
		if (p == &ults[MaxUlts]) {
			return -1;
		}
		else if (p->state == Unused) {
			break;
		}
	}

	stack = malloc(StackSize);
	if (!stack) {
		return -1;
	}

	*(uint64_t *)&stack[StackSize -  8] = (uint64_t)ult_stop;
	*(uint64_t *)&stack[StackSize - 16] = (uint64_t)f;

	p->context.rsp = (uint64_t)&stack[StackSize - 16];
	p->state = Ready;

	return 0;
}


/* Now, let's run some simple threaded code. */

void f(void) {
	static int x;
	int i, id;

	id = ++x;
	for (i = 0; i < 10; i++) {
		printf("%d %d\n", id, i);
		ult_yield();
	}
}

void test() {
	for(int i=0;i<5;i++) {
		puts("soy un ult");
		//ult_yield();
		printf("%d \n", i);
		sleep(1);
		ult_yield();
	}
}

int main(void) {
	ult_init();
	//ult_start(f);
	//ult_start(f);
	ult_start(test);
	ult_start(test);
	ult_return(1);
}
