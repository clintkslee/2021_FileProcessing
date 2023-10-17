/* 20172655 leekangsan copy.c */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
int main(int argc, char* argv[])
{
	int fd1, fd2, check;
	char buf[10];
	fd1=open(argv[1], O_RDONLY);
	fd2=open(argv[2], O_RDWR|O_CREAT|O_TRUNC, 0777);
	while(1)
	{
		check=read(fd1, buf, 10); //read 10 bytes from fd1
		if(check==0) //meet EOF, read() return 0
			break;
		write(fd2, buf, check); //check could be smaller than 10 bytes.
		for(int i=0; i<10; i++){
			buf[i]='\0'; //reset buffer
		}
	}
	close(fd1);
	close(fd2);
	return 0;
}
