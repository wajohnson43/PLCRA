#include <stdlib.h>
#include <sys/ptrace.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

void p_trace(bool attach, pid_t pid);

int main(int argc, char* argv[]){
	bool debug = false;

	//input validation
	if(argc < 3){
		printf("%s <pid> <write>\n", argv[0]);
		exit(-1);
	}

	//get pid
	pid_t pid = (pid_t)atoi(argv[1]);

	//get write
	if(strcmp(argv[2], "0") != 0 && strcmp(argv[2], "1") != 0){
		printf("write must be a boolean value\nQuitting");
		return 0;
	}

	int w = atoi(argv[2]);

	//call ptrace
	p_trace(true, pid);

	//open all the files
	char dumpFileName[16] = "./";
	strcat(dumpFileName, argv[1]);
	strcat(dumpFileName, "_c.dump");
	FILE *dumpFile = fopen(dumpFileName, "w");


	char mapFileName[16] = "/proc/";
	strcat(mapFileName, argv[1]);
	strcat(mapFileName, "/maps");
	FILE *mapFile = fopen(mapFileName, "r");

	char memFileName[16] = "/proc/";
	strcat(memFileName, argv[1]);
	strcat(memFileName, "/mem");
	FILE *memFile = fopen(memFileName, "r");

	if(debug){
		printf("dump file: %s\n", dumpFileName);
		printf("map file: %s\n", mapFileName);
		printf("mem file: %s\n", memFileName);
	}
	//check to see if everything opened...
	if(dumpFile == NULL){
		perror("could not write to dump file\n");
		exit(EXIT_FAILURE);
	}
	if(mapFile == NULL){
		perror("could not open map file\n");
		exit(EXIT_FAILURE);
	}
	if(memFile == NULL){
		perror("could not open mem file\n");
		exit(EXIT_FAILURE);
	}

	//read the important part
	char buf[150];	//map file line
	//unsigned char chunk[7000000];	//memory file chunk
	
	//we need an accumulator to measure how much memory is collected
	long double acc = 0;
	while(fgets(buf, 150 , mapFile) != NULL){

		//split it up, so we can do some math
		char* start_string = strtok(buf, "-");
		char* end_string = strtok(NULL, " ");
		char* perm_string = strtok(NULL, " ");
		char* offset_string = strtok(NULL, " ");
		char* dev_string = strtok(NULL, " ");
		char* inode_string = strtok(NULL, " ");
		char* pathname_string = strtok(NULL, " ");	
		
		//We only want to get executable memory in this case.
		//This omits anything on the stack, heap, virtual system calls
		//or anything declare anonymously	
		if(strcmp(inode_string, "0") == 0)
			continue;

		//printf("start: %s\tend: %s\n", start_string, end_string);
		//printf("perm: %s\toffset: %s\tdev: %s\tinode: %s\n", perm_string, offset_string, dev_string, inode_string);
		//printf("path: %s\n", pathname_string);

		//then get the location: end-start
		unsigned long start = strtol(start_string, NULL, 16);
		unsigned long end = strtol(end_string, NULL, 16);
		unsigned long length = end - start;

		//add the length to the accumulator
		acc += length;
		
		if(w == 1){
			//make the buffer
			unsigned char* chunk = (unsigned char*)malloc(length * sizeof(unsigned char));

			//print memory locations if debug
			if(debug){
				printf("start(hex): %s \t end(hex): %s\n", start_string, end_string);
				printf("start(dec): %lu\t end(dec): %lu\n", start, end);
			}

			//read that location from mem file
			fseek(memFile, (long int)start, SEEK_SET);
			fread(chunk, (long int)length, 1, memFile);
		
			//put it in the dump file
			fwrite(chunk, (long int)length, 1, dumpFile);

			//delete the memory
			free(chunk);

			if(debug){
				printf("length of chunk: %lu\n", length);
			}
		}
	}

	fclose(mapFile);
	fclose(memFile);
	fclose(dumpFile);

	p_trace(false, pid);

	printf("%Lf\n", acc);
	
	return 0;
}


void p_trace(bool attach, pid_t pid){
	
	int op, err;

	if(attach)
		op = 16;
	else
		op = 17;

	err = ptrace(op, pid, NULL, NULL);

	return;
}
