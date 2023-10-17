#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "person.h"

// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, 16, SEEK_SET); //except header record
	fseek(fp, pagenum*PAGE_SIZE, SEEK_CUR);
	fread(pagebuf, PAGE_SIZE, 1, fp);
}

// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다.
void unpack(char *recordbuf, Person *p)
{
	char* cp=strtok(recordbuf, "#");
	strcpy(p->id, cp);
	cp=strtok(NULL, "#");
	strcpy(p->name, cp);
	cp=strtok(NULL, "#");
	strcpy(p->age, cp);
	cp=strtok(NULL, "#");
	strcpy(p->addr, cp);
	cp=strtok(NULL, "#");
	strcpy(p->phone, cp);
	cp=strtok(NULL, "#");
	strcpy(p->email, cp);
}

// 주어진 레코드 파일(recordfp)을 이용하여 심플 인덱스 파일(idxfp)을 생성한다.
void createIndex(FILE *idxfp, FILE *fp)
{
	int numOfPage;
	int numOfRecord; //including deleted records
	int numOfAlive=0; //without deleted records
	int numOfDead=0; //deleted records

	//레코드 파일 내 페이지 개수, 레코드 개수 불러오기
	fseek(fp, 0, SEEK_SET);
	fread(&numOfPage, 4, 1, fp);
	fread(&numOfRecord, 4, 1, fp);
	fseek(fp, 0, SEEK_SET);

	int* arr=(int*)malloc(sizeof(int)*numOfPage); //i번째 page의 record 개수(삭제레코드 포함) 
	char* pagebuf=calloc(PAGE_SIZE, sizeof(char));
	char* recordbuf=calloc(MAX_RECORD_SIZE, sizeof(char));

	Person *p = malloc(sizeof(Person));
	Person *parr = malloc(sizeof(Person)*numOfRecord); //for bubble sort records
	int *pnum = malloc(sizeof(int)*numOfRecord); //page num
	int *rnum = malloc(sizeof(int)*numOfRecord); //record num
	int offset;
	int length;
	int temp;

	for(int i=0; i<numOfPage; i++) //i==페이지번호
	{
		readPage(fp, pagebuf, i); //페이지 읽어오면, 레코드 개수 읽기
		memcpy(&arr[i], pagebuf, 4);

		for(int j=0; j<arr[i]; j++) //각 레코드 읽어오기, j==레코드번호
		{
			memcpy(&offset, pagebuf+(j*8)+4, 4); //j번째 레코드 offset
			memcpy(&length, pagebuf+(j*8)+8, 4); //j번째 레코드 length
			if(*(pagebuf+HEADER_AREA_SIZE+offset)=='*') //삭제 레코드 예외처리 
			{
				numOfDead++;
				continue;
			}
			else //삭제된 레코드 아닐 시 j번째 레코드 recordbuf에 복사
			{
				memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+offset, length);
				unpack(recordbuf, p);
				
				memcpy(&(parr[numOfAlive]), p, sizeof(Person));
				pnum[numOfAlive]=i;
				rnum[numOfAlive]=j;
				numOfAlive++;
			}
		} //for - read record
	} //for - readPage
	
	for(int i=0; i<numOfAlive-1; i++) // ascending sort parr[] (using bubble sort)
	{
		for(int j=0; j<numOfAlive-1-i; j++)
		{
			if(strcmp(parr[j].id, parr[j+1].id)>0)
			{
				
				memcpy(p, &(parr[j]), sizeof(Person));
				memcpy(&(parr[j]), &(parr[j+1]), sizeof(Person));
				memcpy(&(parr[j+1]), p, sizeof(Person));

				temp=pnum[j];
				pnum[j]=pnum[j+1];
				pnum[j+1]=temp;
				
				temp=rnum[j];
				rnum[j]=rnum[j+1];
				rnum[j+1]=temp;
			}
		}
	} //for - bubble sort

	//create index file
	fseek(idxfp, 0, SEEK_SET);
	fwrite(&numOfAlive, 4, 1, idxfp); //index file header - number of records

	char idxid[13];
	for(int i=0; i<numOfAlive; i++)
	{
		for(int j=0; j<13; j++)
			idxid[j]='\0';
		strcpy(idxid, parr[i].id);
		fwrite(idxid, 13, 1, idxfp); //write id
		fwrite(&pnum[i], 4, 1, idxfp); //write page num
		fwrite(&rnum[i], 4, 1, idxfp); //write record num
	}
	
	free(arr);
	free(pagebuf);
	free(recordbuf);
	free(p);
	free(parr);
	free(pnum);
	free(rnum);
	return;
}

// 주어진 심플 인덱스 파일(idxfp)을 이용하여 주민번호 키값과 일치하는 레코드의 주소, 즉 페이지 번호와 레코드 번호를 찾는다.
// 이때, 반드시 이진 검색 알고리즘을 사용하여야 한다.
void binarysearch(FILE *idxfp, const char *id, int *pageNum, int *recordNum)
{
	int numOfAlive;
	char idxid[14]={0,};
	fseek(idxfp, 0, SEEK_SET);
	fread(&numOfAlive, 4, 1, idxfp);

	int low=0;
	int high=numOfAlive-1;
	int mid;
	int reads=0;

	while(low<=high) //binary search
	{
		mid=(low+high)/2;
		fseek(idxfp, 4+mid*21, SEEK_SET);
		fread(idxid, 13, 1, idxfp);
		reads++; //read count
		if(strcmp(idxid, id)==0)
		{
			fread(pageNum, 4, 1, idxfp);
			fread(recordNum, 4, 1, idxfp);
			printf("#reads:%d\n", reads);
			return;
		}
		else if(strcmp(idxid, id)>0)
			high=mid-1;
		else
			low=mid+1;
	}
	printf("#reads:%d\n", reads);
	printf("no persons\n");
	return;
}

int main(int argc, char *argv[])
{
	FILE *fp; //레코드 파일의 파일 포인터
	FILE *idxfp; //simple index file pointer
	char openOption = argv[1][0]; //file open option : i, b
	char id[14] = {0,}; //id buffer, copy from argv[4]
	
	int pageNum = -1;
	int recordNum = -1;
	char* pagebuf=calloc(PAGE_SIZE, sizeof(char));
	int offset, length;
	char* recordbuf=calloc(MAX_RECORD_SIZE, sizeof(char));
	Person *p = malloc(sizeof(Person));

	switch(openOption)
	{
		case 'i': //create index file
			fp=fopen(argv[2], "r"); //open records.dat read only
			idxfp=fopen(argv[3], "w"); //open records.idx write only
			createIndex(idxfp, fp);
			break;

		case 'b': //binary search in index file
			fp=fopen(argv[2], "r"); //open records.dat read only
			idxfp=fopen(argv[3], "r"); //open records.idx read only
			strcpy(id, argv[4]); //get id from user input
			binarysearch(idxfp, id, &pageNum, &recordNum); //find page num and record num 
			if(pageNum==-1 || recordNum==-1) //읽어온 값이 없을 경우 종료
				break;
			readPage(fp, pagebuf, pageNum); //읽어온 pageNum에 해당하는 page 읽어오기
			memcpy(&offset, pagebuf+(recordNum*8)+4, 4); //page로부터 구하려는 레코드 offset 추출
			memcpy(&length, pagebuf+(recordNum*8)+8, 4); //page로부터 구하려는 레코드 length 추출
			memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+offset, length); //사용자 입력에 대응하는 레코드 추출
			unpack(recordbuf, p); //구한 record Person 구조체에 저장
			printf("id=%s\n", p->id);
			printf("name=%s\n", p->name);
			printf("age=%s\n", p->age);
			printf("addr=%s\n", p->addr);
			printf("phone=%s\n", p->phone);
			printf("email=%s\n", p->email);
			break;
	} //switch
	
	free(pagebuf);
	free(recordbuf);
	free(p);
	fclose(fp);
	fclose(idxfp);
	return 0;
} //main
