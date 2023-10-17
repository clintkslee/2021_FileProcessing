//20172655 lee kang san
//read_random.c
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#define SUFFLE_NUM 1000000

void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);

int main(int argc, char **argv)
{
	int *read_order_list; 
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
	read_order_list = (int*)malloc(sizeof(int)*num_of_records);
	
	GenRecordSequence(read_order_list, num_of_records); //make read_order_list 

	gettimeofday(&start_time, NULL);
	for(int i=0; i<num_of_records; i++) //read records randomly
	{
		fseek(fp, 4+(read_order_list[i])*250, SEEK_SET); //calculte offset
		fread(record_buffer, 1, 250, fp);
	}
	gettimeofday(&end_time, NULL);

	elapsed_sec = end_time.tv_sec - start_time.tv_sec;
	elapsed_usec = end_time.tv_usec - start_time.tv_usec;
	elapsed_time = elapsed_sec*1000000 + elapsed_usec; 
//	printf("#start sec: %ld, start usec: %ld\n", start_time.tv_sec, start_time.tv_usec);
//	printf("#end sec: %ld, end usec: %ld\n", end_time.tv_sec, end_time.tv_usec);
//	printf("#sec: %ld usec: %ld\n", elapsed_sec, elapsed_usec);
	printf("#records: %d elapsed_time: %ld us\n", num_of_records, elapsed_time);

	fclose(fp);
	return 0;
}

void GenRecordSequence(int *list, int n)
{
	int i, j, k;

	srand((unsigned int)time(0));

	for(i=0; i<n; i++)
	{
		list[i] = i;
	}
	
	for(i=0; i<SUFFLE_NUM; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}
