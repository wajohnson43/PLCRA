#include <stdlib.h>
#include <sys/ptrace.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>

void p_trace(bool attach, pid_t pid);

int main(int argc, char* argv[]){

	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

	bool debug = false;

	//input validation
	if(argc < 2){
		printf("%s <pid>\n", argv[0]);
		exit(-1);
	}

	//get pid
	pid_t pid = (pid_t)atoi(argv[1]);

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
	unsigned char chunk[7000000];	//memory file chunk
	while(fgets(buf, 150 , mapFile) != NULL){

		//split it up, so we can do some math
		char* start_string = strtok(buf, "-");
		char* end_string = strtok(NULL, "-");
		end_string = strtok(end_string, " ");

		//then get the location: end-start
		unsigned long start = strtol(start_string, NULL, 16);
		unsigned long end = strtol(end_string, NULL, 16);
		unsigned long length = end - start;

		//print memory locations if debub
		//if(debug){
		//	printf("start(hex): %s \t end(hex): %s\n", start_string, end_string);
		//	printf("start(dec): %lu\t end(dec): %lu\n", start, end);
		//}

		//read that location from mem file
		fseek(memFile, (long int)start, SEEK_SET);
		fread(chunk, (long int)length, 1, memFile);
		
		//put it in the dump file
		fwrite(chunk, (long int)length, 1, dumpFile);

		if(debug){
			printf("length of chunk: %lu\n", length);
		}
	}

	fclose(mapFile);
	fclose(memFile);
	fclose(dumpFile);

	p_trace(false, pid);
	
	gettimeofday(&tv2, NULL);

	printf("Time: %ld\n" , ((tv2.tv_sec * 1000000 + tv2.tv_usec) - (tv1.tv_sec * 1000000 + tv1.tv_usec)));

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
