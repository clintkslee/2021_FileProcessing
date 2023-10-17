#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "blockmap.h"

int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);

int* amt; //address mapping table
int* pbn_arr; //[pbn]이 할당 중인지 확인, 사용중 1, 아니면 0 (free block도 사용 중으로 생각)
int free_block_pbn; //free block의 pbn 저장
char* pagebuf; //page 단위 buffer 필요 시 사용

void ftl_open()
{
//printf("=========FTL_OPEN START==============\n");
	int lbn;
	pagebuf=(char*)malloc(sizeof(char)*PAGE_SIZE);
	amt=(int*)malloc(sizeof(int)*DATABLKS_PER_DEVICE); // amt 생성, amt의 lbn 수는 DATABLKS_PER_DEVICE와 동일
	pbn_arr=(int*)malloc(sizeof(int)*BLOCKS_PER_DEVICE); //[phn] block이 lsn에 할당되었는지 확인
	for(int i=0; i<DATABLKS_PER_DEVICE; i++) //amt 초기화(-1 == lsn에 pbn이 지정되지 않은 상태)
		amt[i]=-1;
	for(int i=0; i<BLOCKS_PER_DEVICE; i++) //pbn_arr 초기화(0 == [pbn] block은 lsn에 할당되지 않은 상태)
		pbn_arr[i]=0;
	// flashdrive의 모든page읽어 lbn 추출, -1 아닐 시 amt에 저장 (4개 block이 1pbn 담당)
	for(int i=0; i<(PAGES_PER_BLOCK*BLOCKS_PER_DEVICE); i++)
	{   // 4 page의 lbn이 모두 -1이면 데이터 없는 block 또는 free block
		dd_read(i, pagebuf); //flashdrive의 모든 page 읽기 : ppn 0~63
		memcpy(&lbn, pagebuf+SECTOR_SIZE, 4); //pagebuf에서 lbn위치부터 4바이트 복사 
//printf("ppn : %d readed lbn : %d\n", i, lbn);
		if(lbn!=-1 && (0<=lbn && lbn<=DATABLKS_PER_DEVICE)) //현재 page에 lbn값 존재 시
		{
			int pbn=i/PAGES_PER_BLOCK;
			amt[lbn]=pbn;	// 현재 page 소속된 pbn을 할당
			pbn_arr[pbn]=1;	//  현재 page 소속 pbn 사용중으로 상태 변경
		}
	}
	for(int i=0; i<BLOCKS_PER_DEVICE; i++) //free block 지정 -> 최초로 나온 빈 pbn 로 설정
	{
		if(pbn_arr[i]==0)
		{
			pbn_arr[i]=1; // free block 할당 표기 
			free_block_pbn=i; // free block's pbn 초기화
			break;
		}
	}
	free(pagebuf);
//printf("=========FTL_OPEN END==============\n");
	return;
}

void ftl_read(int lsn, char *sectorbuf)
{
//printf("=========FTL_READ START==============\n");
	int lbn=lsn/PAGES_PER_BLOCK;
	int offset=lsn%PAGES_PER_BLOCK;
	int pbn=amt[lbn];
	if(pbn==-1) //아예 block 할당도 되지 않은 경우
	{
		printf("lsn %d -> lbn %d isn't mapped with pbn, amt[lbn]=pbn is -1\n", lsn, lbn);
		return;
	}
	int ppn=pbn*PAGES_PER_BLOCK+offset;
	pagebuf=(char*)malloc(sizeof(char)*PAGE_SIZE);
	dd_read(ppn, pagebuf);
	memcpy(sectorbuf, pagebuf, SECTOR_SIZE); //pagebuf에서 512바이트 복사하여 sectorbuf에 저장 
	free(pagebuf);
//printf("=========FTL_READ END==============\n");
	return;
}

void ftl_write(int lsn, char *sectorbuf)
{
//	printf("=========FTL_write START==============\n");
	int lbn=lsn/PAGES_PER_BLOCK;
	int offset=lsn%PAGES_PER_BLOCK;
	int pbn=amt[lbn];
	int ppn;
	pagebuf=(char*)malloc(sizeof(char)*PAGE_SIZE);
	memcpy(pagebuf, sectorbuf, SECTOR_SIZE); //pagebuf에 sectorbuf 내용 저장
	memcpy(pagebuf+SECTOR_SIZE, &lbn, 4); //pagebuf에 lbn 저장 
	memcpy(pagebuf+SECTOR_SIZE+4, &lsn, 4); //pagebuf에 lsn 저장 ========== write 할 data 생성 완료 
	if(pbn==-1) //인자로 받은 lsn이 속한 lbn에 pbn 지정x -> pbn 할당 후 write
	{
		int check=0;
//printf("condition1 : pbn is not allocated, lbn = %d, lsn = %d\n", lbn, lsn);
		for(int i=0; i<BLOCKS_PER_DEVICE; i++)
		{
			if(pbn_arr[i]==0) //가장 먼저 오는 미사용 pbn 할당
			{
				check=1;
				pbn_arr[i]=1; //해당 pbn 할당 표시
				amt[lbn]=i;//amt에 lbn-pbn 간 연관 표시
				pbn=amt[lbn];
			}
			if(check==1) break;
		}
		ppn=pbn*PAGES_PER_BLOCK+offset;
		dd_write(ppn, pagebuf);
	}
	else //인자로 받은 lsn이 속한 lbn에 pbn은 지정되어 있음
	{
		int lsntemp; //ppn이 비어있는지 확인 위해 대상 page에서 lsn 추출 
		char* pagetemp = (char*)malloc(sizeof(char)*PAGE_SIZE); 
		ppn=pbn*PAGES_PER_BLOCK+offset;
		dd_read(ppn, pagetemp);
		memcpy(&lsntemp, pagetemp+SECTOR_SIZE+4, 4);
		if(lsntemp==-1) //lbn에 pbn은 할당 되어있지만 해당 ppn에는 최초 write
		{
//printf("condition2 : pbn is allocated, current page first to use, lbn = %d, lsn = %d\n", lbn, lsn);
			dd_write(ppn, pagebuf); //앞서 생성한 data write
		}
		else //해당 ppn을 overwrite -> free block 활용, 기존 pbn은 erase후 free block
		{
//printf("condition3 : overwrite is needed, lbn = %d, lsn = %d\n", lbn, lsn);
			int ppntemp; //현재 pbn내 overwrite할 page제외한 나머지 복사에 사용
			for(int i=0; i<PAGES_PER_BLOCK; i++)
			{
				if(i==offset) continue; //overwrite 위치는 복사x
				ppntemp=pbn*PAGES_PER_BLOCK+i; //pbn내 page들 순회하며 복사
				dd_read(ppntemp, pagetemp);
				dd_write(free_block_pbn*PAGES_PER_BLOCK+i, pagetemp); //free block의 각각의 page에 paste
			}
			dd_write(free_block_pbn*PAGES_PER_BLOCK+offset, pagebuf); //overwrite
			dd_erase(pbn);
			amt[lbn]=free_block_pbn; //free block pbn을 새로운 pbn로 사용
			free_block_pbn=pbn; //기존 pbn을 free block으로 사용
		}
		free(pagetemp);
	}
	free(pagebuf);
//printf("=========FTL_write end==============\n");
	return;
}

void ftl_print()
{
//printf("=========FTL_print start==============\n");
	printf("%5s%5s\n", "lbn", "pbn");
	for(int i=0; i<DATABLKS_PER_DEVICE; i++)
		printf("%5d%5d\n", i, amt[i]);
	printf("free block's pbn = %d\n", free_block_pbn);
//printf("\nftl print end******************************\n");
	return;
}
