/* 20172655 leekangsan delete.c */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char* argv[])
{
	int fd, offset, delete_byte, cnt, check;
	char* buf=(char*)malloc(sizeof(char)); //buffer for counting bytes.
	char*temp; 

	offset=atoi(argv[2]);
	delete_byte=atoi(argv[3]);
	fd=open(argv[1], O_RDONLY);

	cnt=0;
	while(1) //counting how many bytes in file.
	{
		check=read(fd, buf, 1);
		if(check==0) break; //file end
		cnt++;
	}
	temp=(char*)malloc(sizeof(char)*cnt+1);

	lseek(fd, 0, SEEK_SET);
	check=read(fd, temp, cnt); //copy whole bytes and store in temp1
	close(fd);

	for(int i=offset; i<offset+delete_byte; i++)
	{
		if(i>cnt-1)
			break;
		temp[i]='\0';
	}
	strcat(temp, temp+offset+delete_byte);

	fd=open(argv[1], O_RDWR|O_TRUNC); 
	write(fd, temp, strlen(temp)); //write temp on file
	
	free(buf);
	free(temp);
	close(fd);
	return 0;
}
