/* 20172655 leekangsan insert.c */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char* argv[])
{
	int fd, offset, cnt, check;
	char* buf=(char*)malloc(sizeof(char)); //buffer for counting bytes.
	char* temp;

	offset=atoi(argv[2]);

	fd=open(argv[1], O_RDWR|O_CREAT,0777);
	lseek(fd, offset, SEEK_SET);

	cnt=0;
	while(1) //counting how many bytes to shift.
	{
		check=read(fd, buf, 1);
		if(check==0) break; //file end
		cnt++;
	}
	lseek(fd, offset, SEEK_SET);
	temp=(char*)malloc(sizeof(char)*cnt);
	check=read(fd, temp, cnt); //store data for appending.

	lseek(fd, offset, SEEK_SET);
	write(fd, argv[3], strlen(argv[3])); //insert data.

	write(fd, temp, cnt); //append temp.

	free(buf);
	free(temp);
	close(fd);
	return 0;
}
