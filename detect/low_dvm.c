#include <stdlib.h>
#include <sys/ptrace.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void p_trace(bool attach, pid_t pid);

int main(int argc, char* argv[]){
	bool debug = true;

	//input validation
	if(argc < 3){
		printf("%s <pid> <chunk_size>\n", argv[0]);
		exit(-1);
	}

	//get pid
	pid_t pid = (pid_t)atoi(argv[1]);

	//get chunk size
	int chunk_size = atoi(argv[2]);

	//call ptrace
	p_trace(true, pid);

	//open all files
	char dumpFileName[16] = "./";
	strcat(dumpFileName, argv[1]);
	strcat(dumpFileName, "_lowc.dump");
	//this is different, becauwe we're using syscalls
	int dumpDesc = open(dumpFileName, O_CREAT|O_WRONLY);

	if(dumpDesc == -1){
		printf("dump file could not be opened\nquitting...\n");
		exit(EXIT_FAILURE);
	}

	char mapFileName[16] = "/proc/";
	strcat(mapFileName, argv[1]);
	strcat(mapFileName, "/maps");
	//this is dfferent because we're using syscalls
	int mapDesc = open(mapFileName, O_RDONLY);

	if(mapDesc == -1){
		printf("map file could not be opened\nquitting...\n");
		exit(EXIT_FAILURE);
	}

	char memFileName[16] = "/proc/";
	strcat(memFileName, argv[1]);
	strcat(memFileName, "/mem");
	//this is different because we're using syscalls
	int memDesc = open(memFileName, O_RDONLY);

	if(memDesc == -1){
		printf("mem file could not be opened\nquitting...\n");
		exit(EXIT_FAILURE);
	}

	//this loop reads in the memory location
	char buf[150];
	ssize_t amountRead = read(mapDesc, buf,150);
	
	//if the read was successful
	while(amountRead > 0 && strlen(buf) == 150){
		
		//We have to find the newline first!
		char* first_part = strtok(buf, "\n");
		char* next_part = strtok(NULL, "\n");

		while(next_part != NULL){
			//We need to seek back the length of the next_part
			int backDist = -1 * strlen(next_part);
			lseek(mapDesc, backDist, SEEK_CUR);
			next_part = strtok(NULL, "\n");
		}	

		//split up the line from mem, so we can do something usefull with it.
		char* start_string = strtok(first_part, "-");
		char* end_string = strtok(NULL, "-");
		end_string = strtok(end_string, " ");

		//get the actual locations
		unsigned long start = strtol(start_string, NULL, 16);
		unsigned long end = strtol(end_string, NULL, 16);
		int length = end - start;

		if(debug){
			printf("start(hex): %s\tend(hex): %s\n", start_string, end_string);
		}
		//go to the right place
		lseek(memDesc, start, SEEK_SET);

		//create a chunk big enough to hold the memory
		//unsigned char* chunk = (unsigned char*)malloc(length * sizeof(unsigned char));
		
		//figure out how many times to repeat
		int iterations = length / chunk_size;
		unsigned char* chunk = (unsigned char*)malloc(chunk_size * sizeof(unsigned char));
		
		//printf("iterations: %d", iterations);

		for(int i = 0; i < iterations; i++){

			//read it in
			read(memDesc, chunk, chunk_size);
			printf("%s\n", chunk);

			//write it out
			ssize_t error = write(dumpDesc, chunk, chunk_size);

			//check error codes
			if(error == -1){
				printf("error: ");
				if(errno == EAGAIN) printf("EAGAIN\n");
				else if(errno == EWOULDBLOCK) printf("EWOULDBLOCK\n");
				else if(errno == EBADF) printf("EBADF\n");
				else if(errno == EDESTADDRREQ) printf("EDESTADDRREQ\n");
				else if(errno == EDQUOT) printf("EDQUOT\n");
				else if(errno == EFAULT) printf("EFAULT\n");
				else if(errno == EFBIG) printf("EFBIG\n");
				else if(errno == EINTR) printf("EINTR\n");
				else if(errno == EINVAL) printf("EINVAL\n");
				else if(errno == EIO) printf("EIO\n");
				else if(errno == ENOSPC) printf("ENOSPC\n");
				else if(errno == EPERM) printf("EPERM\n");
				else if(errno == EPIPE) printf("EPIPE\n");
			}

	
			//debug stuff
			if(debug){
				printf("length of chunk: %d\n", chunk_size);
			}
		}

		//kill that memory leak
		free(chunk);

		//If there is any remainder, grab it
		int remainder = length % chunk_size;
		if(remainder > 0){
			unsigned char* chunk = (unsigned char*)malloc(remainder * sizeof(unsigned char));

                        //read it in
                        read(memDesc, chunk, remainder);

                        //write it out
                        ssize_t error = write(dumpDesc, chunk, remainder);

			//check error codes
                        if(error == -1){
                                printf("error: ");
                                if(errno == EAGAIN) printf("EAGAIN\n");
                                else if(errno == EWOULDBLOCK) printf("EWOULDBLOCK\n");
                                else if(errno == EBADF) printf("EBADF\n");
                                else if(errno == EDESTADDRREQ) printf("EDESTADDRREQ\n");
                                else if(errno == EDQUOT) printf("EDQUOT\n");
                                else if(errno == EFAULT) printf("EFAULT\n");
                                else if(errno == EFBIG) printf("EFBIG\n");
                                else if(errno == EINTR) printf("EINTR\n");
                                else if(errno == EINVAL) printf("EINVAL\n");
                                else if(errno == EIO) printf("EIO\n");
                                else if(errno == ENOSPC) printf("ENOSPC\n");
                                else if(errno == EPERM) printf("EPERM\n");
                                else if(errno == EPIPE) printf("EPIPE\n");
                        }

                        //kill that memory leak
                        free(chunk);

                        //debug stuff
                        if(debug){
                                printf("length of chunk: %d\n", remainder);
                        }

		}

		//read another line from the map file
		ssize_t amountRead = read(mapDesc, buf, 150);
	}

	close(dumpDesc);
	close(mapDesc);
	close(memDesc);

	p_trace(false, pid);

	return 0;
}

void p_trace(bool attach, pid_t pid){
	int op, err;

	if(attach) op = 16;
	else op = 17;

	err = ptrace(op, pid, NULL, NULL);

	return;
}
