#ifndef __DS__
#define __DS__
#include<stdio.h>
#include"macro.h"
#include"../tool/list.h"
typedef int Status;
typedef unsigned char byte;
typedef struct BLOCK0{
    char identifier[10];
    char info[200];
    unsigned short root;
    int startblock;
    int rootFCB;
}BLOCK0;

typedef struct FCB{
    char name[FILE_NAME_LEN];
    unsigned type:1;
    unsigned status:1;
    unsigned short time;
    unsigned short date;
    unsigned int base;
    unsigned int length;
}FCB;

typedef struct FCBList{
    FCB fcb_entry;
    lslink link;
}FCBList;

typedef struct FATitem{
    signed short item:16;
}FATitem;

typedef struct useropen{
    FCB fcb; 
    char dir[80]; 
    unsigned int count;
    unsigned fcbstate:1;
    unsigned topenfile:1;
    int blocknum;
    int offset_in_block;
}useropen;

typedef struct blockchain{
    signed short blocknum:16;
    lslink link;
}blockchain;
#endif