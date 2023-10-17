/* 20172655 leekangsan merge.c */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
int main(int argc, char* argv[])
{
	int fd1, fd2, fd3, check;
	char buf[10];
	fd1=open(argv[1], O_RDWR|O_CREAT|O_TRUNC, 0777);
	fd2=open(argv[2], O_RDONLY);
	fd3=open(argv[3], O_RDONLY);
	while(1) //copy file 2 -> file 1
	{
		check=read(fd2, buf, 10);
		if(check==0)
			break;
		write(fd1, buf, check);
		for(int i=0; i<10; i++){
			buf[i]='\0'; 
		}
	}
	while(1) // append file 3 -> file 1
	{
		check=read(fd3, buf, 10);
		if(check==0)
			break;
		write(fd1, buf, check);
		for(int i=0; i<10; i++){
			buf[i]='\0'; 
		}
	}
	close(fd1);
	close(fd2);
	close(fd3);
	return 0;
}
