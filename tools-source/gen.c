#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#define true 1
#define false 0
#define LOG_PATH	"test_metadata.txt"

void usage(){
	printf("Usage: gen N [-false]\n");
	printf("       gen -d [test]\n\n");
}

void genBuffer(unsigned long N, int flag, int **_buffer, unsigned long *_buffersize){
	// Allocate memory
	unsigned long size = (unsigned long)(N*N*sizeof(int)); 
	int *buffer = (int*)malloc(size);
	
	
	// Use this array to store the sum of each line
	long *sum = (long*)malloc(sizeof(long)*N);
	
	
	// Generate numbers
	unsigned long i, j;
	unsigned int mask = 0x7FFFFFFF/(N*N) & 0x7FF;
	for(i = 0; i < N; i++){
		sum[i] = 0;
		for(j = 0; j < N; j++){
			buffer[(i*N)+j] = ((unsigned int)rand() * (i ^ j)) & mask ;
			sum[i] += buffer[(i*N)+j];
			
			// Add sign
			buffer[(i*N)+j] *= (rand() % 3 == 0) ? -1 : 1; 
		}
		
		// Manually set A[ij] to either larger or smaller than the sum of its respective line
		buffer[(i*N)+i] = flag ? (sum[i] + (rand() % 0x3C) + 5) : sum[i]/2;
		// Add sign
		buffer[(i*N)+i] *= (rand() % 3 == 0) ? -1 : 1; 
	}
	
	// Pass the buffer to main
	*_buffersize = size;
	*_buffer = buffer;
	
	free(sum);
}

int writeFile(int *_buffer, unsigned long _N, char _flag){
	FILE *fp;
	char filename[13];
	char pid_c[5];
	int fileid = getpid()%1000;
	
	strcpy(filename, "test/");
	sprintf(pid_c, "%d", fileid);
	
	strcat(filename, pid_c);
	
	unsigned long zero = 0;
	fp = fopen((const char*)filename, "wb");
	if(fp!=NULL){
		fwrite(&_N, sizeof(long), 1, fp);
		fwrite(&_flag, sizeof(char), 1, fp);
		fwrite(&zero, sizeof(char), 7, fp);
		fwrite(_buffer, sizeof(int), _N*_N, fp); 

	}
	else{
		printf("Error opening output file\n");
		return 1000;
	}
	fclose(fp);
	
	return fileid;
}

void readFile(const char *test_no){
	FILE *fp;
	char filename[13];
	
	strcpy(filename, "test/");
	strcat(filename, test_no);
	
	char flag;
	unsigned long N;
	int *A;
	fp = fopen((const char*)filename, "rb");
	if(fp != NULL){
		fread(&N, sizeof(long), 1, fp);
		fread(&flag, sizeof(char), 1, fp);
		fseek(fp, 7, SEEK_CUR);
		A = malloc(sizeof(int) * N * N);
		fread(A, sizeof(int), N*N, fp);
		fclose(fp);
		
		printf("Test %s: %lu Numbers (%lux%lu)", test_no, N*N, N, N);
		if(flag)
			printf(" Should pass\n");
		else
			printf(" Should not pass\n");
			
		unsigned long i, o;
		for(i = 0; i < N; i++){
			for(o = 0; o < N; o++){
				if(i != o)
					printf(" %d  ", A[i*N+o]);
				else
					printf("[%d] ", A[i*N+o]);
			}
			printf("\n");
		}
		free(A);
		
	}
	else{
		printf("No such file was found\n\n");
	}
}

void writeLog(int _id, unsigned long _n, char _flag){
	FILE *log;
	log = fopen(LOG_PATH, "a");
	if(log == NULL){
		exit(1);
	}
	
	fprintf(log, "%d\t\t%lu\t(%lux%lu)\t\t%d\n", _id, _n*_n, _n, _n, _flag);
	fclose(log);
} 


int main(int argc, char **argv){
	srand((unsigned int)(getpid() ^ 0x0E56D0A7));
	
	char flag;
	unsigned long N;
	unsigned long bufferSize;
	int *buffer;
	
	if(argc == 1){
		N = (rand() % 0x800);
		printf("Generating %lux%lu matrix (%lu numbers)\n", N, N, N*N);
		genBuffer(N, true, &buffer, &bufferSize);
		flag = 0xFF;
	}
	else if(argc == 2 && !strcmp(argv[1], "--help")){
		usage();
		return 0;
	}
	else if(argc == 2){
		N = strtoul(argv[1], NULL, 0); // const char* to unsigned long
		if(N > 0x4000){
			printf("gen: N=%s out of limits; Must be smaller than 16384 (0x4000)\n\n", argv[1]);
		}
		printf("Generating %lux%lu matrix (%lu numbers)\n", N, N, N*N);
		genBuffer(N, true, &buffer, &bufferSize);
		flag = 0xFF;
	
	}
	else if(argc == 3 && !strcmp(argv[1], "-d")){
		readFile(argv[2]);
		return 0;
	}
	else if(argc == 3 && !strcmp(argv[2], "-false")){
		N = strtoul(argv[1], NULL, 0); // const char* to unsigned long
		if(N > 0x4000){
			printf("gen: N=%s out of limits; Must be smaller than 16384 (0x4000)\n\n", argv[1]);
		}
		printf("Generating %lux%lu matrix (%lu numbers)\n", N, N, N*N);
		genBuffer(N, false, &buffer, &bufferSize);
		flag = 0;
		
	}
	else{
		usage();
		return 0;
	}
	
	int fileid = writeFile(buffer, N, flag);
	writeLog(fileid, N, flag);
	printf("Created %d\n\n", fileid);
	free(buffer);

	return 0;
}
