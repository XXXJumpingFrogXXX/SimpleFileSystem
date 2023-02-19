#include<stdio.h>
#include"api/api.h"
#include"global/ds.h"
#include"tool/disk.h"
#include"shell/shell.h"
#include"tool/time.h"
char sysname[20]="[2012289-disk]";
char pwd[80];
FILE * DISK;
BLOCK0 block0;
FATitem FAT1[FAT_ITEM_NUM];
FATitem FAT2[FAT_ITEM_NUM];
FCB presentFCB; 
useropen uopenlist[MAX_FD_NUM];
char * type[2]={"file","directory"};
int main()
{
    init_system();
    go();
    return 0;
}