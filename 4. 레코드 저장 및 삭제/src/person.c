#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "person.h"

// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다

// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 위의 readPage() 함수를 호출하여 pagebuf에 저장한 후, 여기에 필요에 따라서 새로운 레코드를 저장하거나
// 삭제 레코드 관리를 위한 메타데이터를 저장합니다. 그리고 난 후 writePage() 함수를 호출하여 수정된 pagebuf를
// 레코드 파일에 저장합니다. 반드시 페이지 단위로 읽거나 써야 합니다.
//
// 주의: 데이터 페이지로부터 레코드(삭제 레코드 포함)를 읽거나 쓸 때 페이지 단위로 I/O를 처리해야 하지만,
// 헤더 레코드의 메타데이터를 저장하거나 수정하는 경우 페이지 단위로 처리하지 않고 직접 레코드 파일을 접근해서 처리한다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, 16, SEEK_SET); //except header record 
	fseek(fp, pagenum*PAGE_SIZE, SEEK_CUR);
	fread(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 레코드 파일의 위치에 저장한다. 
// 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, 16, SEEK_SET); //header record 
	fseek(fp, pagenum*PAGE_SIZE, SEEK_CUR);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}

//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 
// 
void pack(char *recordbuf, const Person *p)
{
	strcat(recordbuf, p->id); strcat(recordbuf, "#");
	strcat(recordbuf, p->name); strcat(recordbuf, "#");
	strcat(recordbuf, p->age); strcat(recordbuf, "#");
	strcat(recordbuf, p->addr); strcat(recordbuf, "#");
	strcat(recordbuf, p->phone); strcat(recordbuf, "#");
	strcat(recordbuf, p->email); strcat(recordbuf, "#");
}

// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다.
//
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

//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값들을 구조체에 저장한 후 아래 함수를 호출한다.
//
void check_header_record(FILE *fp, int *data_page_cnt, int *all_record_cnt, int *page_num, int *record_num)
{
	fseek(fp, 0, SEEK_SET);
	fread(data_page_cnt, 4, 1, fp);
	fread(all_record_cnt, 4, 1, fp);
	fread(page_num, 4, 1, fp);
	fread(record_num, 4, 1, fp);
	fseek(fp, 0, SEEK_SET);
}
void add(FILE *fp, const Person *p)
{
	printf("*** add() called ***\n");
	char* recordbuf=(char*)calloc(MAX_RECORD_SIZE, sizeof(char));
	pack(recordbuf, p); //data packing
	
	//header record 확인
	int data_page_cnt, all_record_cnt, page_num, record_num;
	check_header_record(fp, &data_page_cnt, &all_record_cnt, &page_num, &record_num);

	//pagebuf 생성
	char* pagebuf=(char*)calloc(PAGE_SIZE, sizeof(char));

	//최초 저장 시
	if(data_page_cnt==0 && all_record_cnt==0)
	{
		int records, offset, length;
		records=1;
		offset=0;
		length=strlen(recordbuf);

		data_page_cnt++;
		all_record_cnt++;
		
		fseek(fp, 0, SEEK_SET);
		fwrite(&data_page_cnt, 4, 1, fp);
		fwrite(&all_record_cnt, 4, 1, fp);
		
		memcpy(pagebuf+0, &records, 4);
		memcpy(pagebuf+4, &offset, 4);
		memcpy(pagebuf+8, &length, 4); //page의 header area
		memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, length); //첫번째 record
		
		writePage(fp, pagebuf, 0);
	}
	else
	{
		if(page_num==-1 && record_num==-1) //삭제 레코드 없을 경우
		{
			readPage(fp, pagebuf, data_page_cnt-1); //마지막 page read
			int records, length, addlength=0;
			memcpy(&records, pagebuf, 4); //마지막 page의 레코드 수
			for(int i=0; i<records; i++) //현재 page 레코드 개수 만큼 반복
			{
				memcpy(&length, pagebuf+8+i*8, 4); //마지막 page의 데이터 길이
				addlength+=length;
			}
			length=strlen(recordbuf); //작성할 record의 길이
			//마지막 page에 append할 공간 여유 있나?
			if((HEADER_AREA_SIZE-(4+records*8))>8 && (DATA_AREA_SIZE-addlength)>length)
			{
				records++;
				memcpy(pagebuf, &records, 4); //레코드 개수 1 증가
				memcpy(pagebuf+4+8*(records-1), &addlength, 4);//offset 설정
				memcpy(pagebuf+4+8*(records-1)+4, &length, 4);//길이
				memcpy(pagebuf+HEADER_AREA_SIZE+addlength, recordbuf, length);
				writePage(fp, pagebuf, data_page_cnt-1);
				all_record_cnt++;
				fseek(fp, 4, SEEK_SET);
				fwrite(&all_record_cnt, 4, 1, fp);
			}
			else //없으면 새 page에 저장
			{
				data_page_cnt++;
				all_record_cnt++;
				fseek(fp, 0, SEEK_SET);
				fwrite(&data_page_cnt, 4, 1, fp);
				fwrite(&all_record_cnt, 4, 1, fp); //header record 수정
				
				free(pagebuf);
				pagebuf=(char*)calloc(PAGE_SIZE, sizeof(char));
				records=1;
				int offset=0;
				length=strlen(recordbuf);
				memcpy(pagebuf+0, &records, 4);
				memcpy(pagebuf+4, &offset, 4);
				memcpy(pagebuf+8, &length, 4); //page의 header area
				memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, length);

				writePage(fp, pagebuf, data_page_cnt-1);
			}
		}
		else //삭제 레코드 있는 경우
		{
			char temprecord[MAX_RECORD_SIZE]={0,};
			//recordbuf 저장할 데이터 들어있음, pagebuf 초기화 상태
			int right_size=0;
			int length=strlen(recordbuf);
			int nextpage=page_num, nextrecord=record_num, delrecordlength, delrecordoffset;
			int beforepage=-1, beforerecord=-1;
			while(nextpage!=-1)//헤더레코드의 값을 시작으로 -1 -1 나올 때까지 탐색
			{
				readPage(fp, pagebuf, nextpage); //삭제된 레코드 페이지 읽어오기 
				memcpy(&delrecordoffset, pagebuf+4+8*record_num, 4);//삭제된 레코드의 오프셋 읽어오기
				memcpy(&delrecordlength, pagebuf+4+8*record_num+4, 4);//삭제된 레코드의 크기 읽어오기
				if(delrecordlength>=length) //적당한 크기의 삭제레코드 발견 시 break
				{
					right_size=1;
					break;
				}
				beforepage=nextpage;
				beforerecord=nextrecord;
				memcpy(temprecord, pagebuf+HEADER_AREA_SIZE+delrecordoffset, delrecordlength);
				memcpy(&nextpage, temprecord+1, 4);//다음 삭제된 레코드의 페이지 읽어오기
				memcpy(&nextrecord, temprecord+5, 4);//다음 삭제된 레코드의 번호 읽어오기
			}
			if(right_size) //적당한 크기의 삭제 레코드 존재 시 nextpage, nextrecord 위치에 recordbuf 저장
			{
				if(beforepage==-1) //첫번째 삭제레코드에 저장 시
				{	
					int temppage, temprecordnum;
					readPage(fp, pagebuf, nextpage);
					memcpy(temprecord, pagebuf+HEADER_AREA_SIZE+delrecordoffset, delrecordlength);
					memcpy(&temppage, temprecord+1, 4);
					memcpy(&temprecordnum, temprecord+5, 4);
					fseek(fp, 8, SEEK_SET);
					fwrite(&temppage, 4, 1, fp);
					fwrite(&temprecordnum, 4, 1, fp); //헤더레코드 수정
					memcpy(pagebuf+HEADER_AREA_SIZE+delrecordoffset, recordbuf, length);	
					writePage(fp, pagebuf, nextpage);
				}
				else //중간, 마지막 삭제레코드에 저장시
				{
					char tempbeforepage[PAGE_SIZE];
					int beforeoffset, beforelength;
					int tempint;

					readPage(fp, tempbeforepage, beforepage);
					memcpy(&beforeoffset, tempbeforepage+4+8*beforerecord, 4);//직전 레코드의 오프셋 읽어오기
					readPage(fp, pagebuf, nextpage); //delrecordoffset, delrecordlength

					memcpy(&tempint, pagebuf+HEADER_AREA_SIZE+delrecordoffset+1, 4);
					memcpy(tempbeforepage+HEADER_AREA_SIZE+beforeoffset+1, &tempint, 4);
					memcpy(&tempint, pagebuf+HEADER_AREA_SIZE+delrecordoffset+5, 4);
					memcpy(tempbeforepage+HEADER_AREA_SIZE+beforeoffset+5, &tempint, 4);

					writePage(fp, tempbeforepage, beforepage);
					memcpy(pagebuf+HEADER_AREA_SIZE+delrecordoffset, recordbuf, length);
					writePage(fp, pagebuf, nextpage);		
				}
			}
			else //적당한 크기의 삭제 레코드 없는 경우 append, 헤더레코드 값 수정
			{
				readPage(fp, pagebuf, data_page_cnt-1); //마지막 page read
				int records, length, addlength=0;
				memcpy(&records, pagebuf, 4); //마지막 page의 레코드 수
				for(int i=0; i<records; i++) //현재 page 레코드 개수 만큼 반복
				{
					memcpy(&length, pagebuf+8+i*8, 4); //마지막 page의 데이터 길이
					addlength+=length;
				}
				length=strlen(recordbuf); //작성할 record의 길이
				//마지막 page에 append할 공간 여유 있나?
				if((HEADER_AREA_SIZE-(4+records*8))>8 && (DATA_AREA_SIZE-addlength)>length)
				{
					records++;
					memcpy(pagebuf, &records, 4); //레코드 개수 1 증가
					memcpy(pagebuf+4+8*(records-1), &addlength, 4);//offset 설정
					memcpy(pagebuf+4+8*(records-1)+4, &length, 4);//길이
					memcpy(pagebuf+HEADER_AREA_SIZE+addlength, recordbuf, length);
					writePage(fp, pagebuf, data_page_cnt-1);
					all_record_cnt++;
					fseek(fp, 4, SEEK_SET);
					fwrite(&all_record_cnt, 4, 1, fp);
				}
				else //공간 없을 시 새 page에 저장
				{
				data_page_cnt++;
				all_record_cnt++;
				fseek(fp, 0, SEEK_SET);
				fwrite(&data_page_cnt, 4, 1, fp);
				fwrite(&all_record_cnt, 4, 1, fp); //header record 수정
				
				free(pagebuf);
				pagebuf=(char*)calloc(PAGE_SIZE, sizeof(char));
				records=1;
				int offset=0;
				length=strlen(recordbuf);
				memcpy(pagebuf+0, &records, 4);
				memcpy(pagebuf+4, &offset, 4);
				memcpy(pagebuf+8, &length, 4); //page의 header area
				memcpy(pagebuf+HEADER_AREA_SIZE, recordbuf, length);

				writePage(fp, pagebuf, data_page_cnt-1);
				}
			}
		}
	}
	free(pagebuf);
	free(recordbuf);
}

//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *id)
{
	printf("*** delete() called ***\n");

	Person *p=(Person*)malloc(sizeof(Person));
	char* pagebuf=(char*)calloc(PAGE_SIZE, sizeof(char));
	char* recordbuf=(char*)calloc(MAX_RECORD_SIZE, sizeof(char));
	int isFound=0, delpage, delrecord;
	int records, offset, length; 

	//header record 불러오기
	int data_page_cnt, all_record_cnt, page_num, record_num; //각 페이지를 레코드 개수 만큼 loop하여 삭제 대상 존재 판단
	check_header_record(fp, &data_page_cnt, &all_record_cnt, &page_num, &record_num);
	if(data_page_cnt <= 0)
	{
		printf("Nothing has been written.\n");
		free(p);
		free(pagebuf);
		free(recordbuf);
		return;
	}
	for(int i=0; i<data_page_cnt; i++) //모든 페이지 탐색
	{
		readPage(fp, pagebuf, i);
		memcpy(&records, pagebuf, 4); //각 페이지의 레코드 개수
		for(int j=0; j<records; j++) //레코드 개수 만큼 탐색
		{
			memcpy(&offset, pagebuf+4+8*j, 4);
			memcpy(&length, pagebuf+4+8*j+4, 4);
			memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+offset, length);
			if(recordbuf[0]=='*') //현재 불러온 레코드가 이미 삭제된 레코드 시 continue
				continue;
			unpack(recordbuf, p);
			if(strcmp(p->id, id)==0)
			{
				delpage=i;
				delrecord=j;
				isFound=1;
				break;
			}
		}
		if(isFound==1)
			break;
	}
	if(isFound==1) //delpage, delrecord에 대응하는 record *표시
	{
		int before_page, before_record;
		free(pagebuf);
		free(recordbuf);
		pagebuf=(char*)calloc(PAGE_SIZE, sizeof(char));
		readPage(fp, pagebuf, delpage);
		memcpy(recordbuf, pagebuf+HEADER_AREA_SIZE+offset, length);
		
		if(page_num==-1 && record_num==-1) //최초 삭제
		{
			before_page=before_record=-1;
			memcpy(recordbuf, "*", 1); //deletion mark	
			memcpy(recordbuf+1, &before_page, 4);
			memcpy(recordbuf+5, &before_record, 4);
			memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, length);
			writePage(fp, pagebuf, delpage);
			fseek(fp, 8, SEEK_SET); //헤더레코드 최근에 삭제된 레코드 변경
			fwrite(&delpage, 4, 1, fp);
			fwrite(&delrecord, 4, 1, fp);	
		}
		else
		{
			before_page=page_num;
			before_record=record_num;
			memcpy(recordbuf, "*", 1); //deletion mark	
			memcpy(recordbuf+1, &before_page, 4);
			memcpy(recordbuf+5, &before_record, 4);
			memcpy(pagebuf+HEADER_AREA_SIZE+offset, recordbuf, length);
			writePage(fp, pagebuf, delpage);
			fseek(fp, 8, SEEK_SET); //헤더레코드 최근에 삭제된 레코드 변경
			fwrite(&delpage, 4, 1, fp);
			fwrite(&delrecord, 4, 1, fp);	
		}
	}
	else //입력 받은 id와 일치하는 레코드 없음
		printf("No record matched.\n");

	free(p);
	free(pagebuf);
	free(recordbuf);
}

int main(int argc, char *argv[])
{
	int check=access("person.dat", F_OK); //person.dat 존재 여부 확인
	FILE *fp;  //레코드파일의 파일 포인터
	if(check==0)
	{
		printf("*** Open data file ***\n");
		fp = fopen("person.dat", "r+");
	}
	else	
	{
		printf("*** Create data file ***\n");
		fp = fopen("person.dat", "w+");

	}
	
	if(check!=0) //만약 레코드 파일 생성 시 header record 초기화
	{
		int temp = 0; //data page, records
		int temp2 = -1; //page num, record num
		fwrite(&temp, 4, 1, fp);
		fwrite(&temp, 4, 1, fp);
		fwrite(&temp2, 4, 1, fp);
		fwrite(&temp2, 4, 1, fp);
		printf("*** Header record setup complete ***\n");
		fseek(fp, 0, SEEK_SET); 
	}

	//임시구조체에 입력 데이터 저장

	//add, delete 분기점
	Person *p=(Person*)malloc(sizeof(Person));
	switch(argv[1][0])
	{
		case 'a': //데이터 저장, add() 호출
			strcpy(p->id, argv[3]);
			strcpy(p->name, argv[4]);
			strcpy(p->age, argv[5]);
			strcpy(p->addr, argv[6]);
			strcpy(p->phone, argv[7]);
			strcpy(p->email, argv[8]);
			add(fp, p);
			break;
		case 'd': //데이터 삭제, delete() 호출
			delete(fp, argv[3]);
			break;
	}
	free(p);
	fclose(fp);
	return 1;
}
