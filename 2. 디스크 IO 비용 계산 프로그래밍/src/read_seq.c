//20172655 lee kang san
//read_seq.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
int main(int argc, char **argv)
{
	int num_of_records;
	FILE* fp;
	char record_buffer[251];

	struct timeval start_time, end_time;
	long elapsed_sec;
	long elapsed_usec;
	long elapsed_time;

	fp=fopen(argv[1], "rt");

	fread(&num_of_records, 4, 1, fp);
	record_buffer[250]='\0';
	
	gettimeofday(&start_time, NULL);
	for(int i=0; i<num_of_records; i++) //read records sqeuntially
		fread(record_buffer, 1, 250, fp);
	gettimeofday(&end_time, NULL);
	
	elapsed_sec = end_time.tv_sec - start_time.tv_sec;
	elapsed_usec = end_time.tv_usec - start_time.tv_usec;
	elapsed_time = elapsed_sec*1000000 + elapsed_usec; 
	//printf("#start sec: %ld, start usec: %ld\n", start_time.tv_sec, start_time.tv_usec);
	//printf("#end sec: %ld, end usec: %ld\n", end_time.tv_sec, end_time.tv_usec);
	//printf("#sec: %ld usec: %ld\n", elapsed_sec, elapsed_usec);
	printf("#records: %d elapsed_time: %ld us\n", num_of_records, elapsed_time);

	fclose(fp);
	return 0;
}
