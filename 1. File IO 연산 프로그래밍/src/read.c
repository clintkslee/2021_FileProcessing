/* 20172655 leekangsan read.c */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
int main(int argc, char* argv[])
{
	int fd, 
		offset, 
		read_byte, //var for iteration
		check; //check read()
	char* buf;
	buf=(char*)malloc(sizeof(char)); //read 1 byte
	fd=open(argv[1], O_RDONLY);	
	
	read_byte=atoi(argv[3]);
	offset=atoi(argv[2]);
	lseek(fd, offset, SEEK_SET); 
	
	for(int i=0; i<read_byte; i++)
	{	
		check=read(fd, buf, 1);
		if(check==0)
			break; //if read() meet EOF, break
		printf("%c", buf[0]);
	}
	close(fd);
	return 0;
}
