#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ptrace.h>
#include <unistd.h>


void sleepfunc(){
	struct timespec* sleeptime = malloc(sizeof(struct timespec));
	sleeptime->tv_sec = 1;
	sleeptime->tv_nsec = 0;

	while(1){
		printf("%d\n", getpid());
		nanosleep(sleeptime, NULL);
	}

	free(sleeptime);
}

int main(){
	sleepfunc();
	return 0;
}
