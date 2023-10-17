//20172655 lee kang san
//create_file.c
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
	int num_of_records;
	FILE* fp;
	char record_buffer[250];
	
	num_of_records=atoi(argv[1]);
	fp=fopen(argv[2], "wt");
	fwrite(&num_of_records, 4, 1, fp); //add header
	
	for(int i=0; i<250; i++) //set record data
		record_buffer[i]='_';
	record_buffer[0]='o';
	record_buffer[249]='x';

	for(int i=0; i<num_of_records; i++) //write records
		fwrite(record_buffer, 1, 250, fp);

	fclose(fp);
	return 0;
}
