/* 20172655 leekangsan overwrite.c */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char* argv[])
{
	int fd, offset;
	offset=atoi(argv[2]);
	fd=open(argv[1], O_RDWR|O_CREAT,0777);
	lseek(fd, offset, SEEK_SET);
	write(fd, argv[3], strlen(argv[3]));
	close(fd);
	return 0;
}
