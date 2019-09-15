
#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>

//
//-O3 option possibly causes optimizations, which make cases with indexes 
// not fitting in 1 byte not handle-able as far as I understand
void* base_ptr;

void my_handler(int sig, siginfo_t *info, void *ctx) {
	static int calls_num = 0;
	ucontext_t *uc = (ucontext_t *)ctx;

	uint64_t command = *(uint64_t *)uc->uc_mcontext.gregs[REG_RIP];
	uint64_t b4_b0_diff = ((uint64_t)uc->uc_mcontext.gregs[REG_RBP] - (uint64_t)base_ptr)>> 2;

	//command id byte 
	if ((command & 0xff) != 0x8b) {
		printf("Unknown segfault\n");
		exit(1);
	}

	//possible indicators
	char has_offset = (command >> 14) & 1;
	char printf_2nd_arg = (command >> 11) & 1;
	char is_b0 = (command >> 9) & 1;

	uint64_t index = has_offset * ((command >> 16) & 0xff) >> 2;
	
	index += (1-is_b0) * b4_b0_diff;

	if (printf_2nd_arg) {
		uc->uc_mcontext.gregs[REG_RCX] = 100000 + 1000 * index + (++calls_num);
	}
	else {
		uc->uc_mcontext.gregs[REG_RDX] = 100000 + 1000 * index + (++calls_num);
	}
	
	uc->uc_mcontext.gregs[REG_RIP] += 2 + has_offset;
}

void init(void *base){
	base_ptr = base;
	struct sigaction act = {
		.sa_sigaction = my_handler,
		.sa_flags = SA_RESTART,
	};
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGSEGV, &act, NULL) == -1) {
		perror("signal set failed");
		exit(1);
	}
}
