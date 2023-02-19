#include<stdio.h>
#include<string.h>
#include"disk.h"
#include"../global/var.h"
#include"list.h"
#include"time.h"
void createDisk(){
    char buff[BLOCK_SIZE]={'0'};
    FILE *f = fopen(sysname,"r+");
    for(int i=0;i<BLOCK_NUMS;i++)
        fwrite(buff,sizeof(buff),1,f);
    fclose(f);
}

//把FCB(ptr数组)写入对应某一个磁盘内存块对应的空FCB分区
int writeToDisk(FILE* DISK,void *ptr,int size,int base,long offset){
    //base对应磁盘块开始的字节位置 offset对应t第一个空的FCB分区开始的字节位置
    if(base<0)
        fseek(DISK,offset,SEEK_SET);
    else
        fseek(DISK,base+offset,SEEK_SET);
    fwrite(ptr,size,1,DISK);
    return 0;
}

//从某一个磁盘内存块中读出所有FCB分区数据放入buff
int readFromDisk(FILE* DISK,void *buff,int size,int base,long offset){
    //size 要读的数据大小 base对应磁盘块开始的字节位置
    if(base<0)
        fseek(DISK,offset,SEEK_SET);
    else
        fseek(DISK,base+offset,SEEK_SET);
    fread(buff,size,1,DISK);
    return 0;
}

//将对应字节位置的FAT表赋值给fat
int getFAT(FATitem * fat,int fat_location){
    readFromDisk(DISK,fat,FAT_ITEM_SIZE*FAT_ITEM_NUM,fat_location,0);
    return 0;
}

//在磁盘中进行修改FAT表
int changeFAT(FATitem *fat,int fat_location){
    writeToDisk(DISK,fat,FAT_ITEM_SIZE*FAT_ITEM_NUM,fat_location,0);
    return 0;
}

//重载FAT
void reloadFAT(){
    getFAT(FAT1,FAT1_LOCATON);
    getFAT(FAT2,FAT2_LOCATON);
}

//重写FAT表
void rewriteFAT(){
    changeFAT(FAT1,FAT1_LOCATON);
    changeFAT(FAT2,FAT2_LOCATON);
}

//初始化一个磁盘内存块（FCB块） blocknum-将要初始化的块号 parentblocknum-父目录块号
int initFCBBlock(int blocknum,int parentblocknum){
//修改FAT
    FAT1[blocknum].item=FCB_BLOCK;
    FAT2[blocknum].item=FCB_BLOCK;
    rewriteFAT();
//加入.和..目录至指定磁盘内存块
//.目录
    struct tm *t=getTimeStruct();
    FCB fcb;
    strcpy(fcb.name,".");
    fcb.type=1;
    fcb.status=USED;
    fcb.time=getTime(t);
    fcb.date=getDate(t);
    fcb.base=blocknum;//指向当前目录
    fcb.length=1;
    addFCB(fcb,blocknum);
//..目录
    strcpy(fcb.name,"..");
    fcb.type=1;
    fcb.status=USED;
    fcb.time=getTime(t);
    fcb.date=getDate(t);
    fcb.base=parentblocknum;//指向父目录
    fcb.length=1;
    addFCB(fcb,blocknum);
    return 0;
}

//把FCB加入（写入）对应磁盘内存块的对应FCB分区
int addFCB(FCB fcb,int blocknum){
    int offset = getEmptyFCBOffset(blocknum);
    if(offset==-1)
        return -1;
    else{
        writeToDisk(DISK,&fcb,sizeof(FCB),blocknum*BLOCK_SIZE,offset*FCB_SIZE);
        return 0;
    }
}

//根据名字在当前目录块寻找对应(已被使用的)FCB
int findFCBInBlockByName(char *name,int blocknum){
    FCB fcb[FCB_ITEM_NUM];
    int offset=-1;
    readFromDisk(DISK,fcb,sizeof(FCB)*FCB_ITEM_NUM,blocknum*BLOCK_SIZE,0);
    for(int i=0;i<FCB_ITEM_NUM;i++){
        if(fcb[i].status==USED){
            if(strcmp(fcb[i].name,name)==0)
            offset=i;
        }
    }
    return offset;
}

int changeFCB(FCB fcb,int blocknum,int offset_in_block){
    writeToDisk(DISK,&fcb,sizeof(FCB),blocknum*BLOCK_SIZE,offset_in_block*FCB_SIZE);
    return 0;
}

int removeFCB(int blocknum,int offset_in_block){
    FCB fcb;
    readFromDisk(DISK,&fcb,sizeof(FCB),blocknum*BLOCK_SIZE,offset_in_block*FCB_SIZE);
    fcb.status=0;
    writeToDisk(DISK,&fcb,sizeof(FCB),blocknum*BLOCK_SIZE,offset_in_block*FCB_SIZE);
    return 0;
}

//返回某一磁盘内存块中第一个空FCB分区的索引（偏移量） -1代表找不到空的FCB分区
int getEmptyFCBOffset(int blocknum){
    FCB fcblist[FCB_ITEM_NUM];
    readFromDisk(DISK,fcblist,sizeof(FCB)*FCB_ITEM_NUM,blocknum*BLOCK_SIZE,0);
    int i;
    for(i=0;i<FCB_ITEM_NUM;i++)
        if(fcblist[i].status==FREE)
            return i;
    if(i==FCB_ITEM_NUM)
        return -1;
}

//将指定磁盘文件块中的指定FCB分区中的FCB赋值给fcb
int getFCB(FCB *fcb,int blocknum,int offset_in_block){
    //offset对应FCB分区开始的块号
    readFromDisk(DISK,fcb,sizeof(FCB),blocknum*BLOCK_SIZE,offset_in_block*FCB_SIZE);
    return 0;
}

int getFCBList(int blocknum,FCBList FLstruct,lslink *fcblisthead){
    FCB fcblist[FCB_ITEM_NUM];
    readFromDisk(DISK,&fcblist,sizeof(FCB)*FCB_ITEM_NUM,blocknum*BLOCK_SIZE,0);
    list_init(fcblisthead,&FLstruct);
    for(int i=0;i<FCB_ITEM_NUM;i++){
        if(fcblist[i].status==USED){
            FCBList *temp = get_node(FCBList);
            temp->fcb_entry = fcblist[i];
            list_insert(fcblisthead,&(temp->link),temp);
        }  
    }
}

int getFCBNum(int blocknum){
    int num=0;
    FCB fcblist[FCB_ITEM_NUM];
    readFromDisk(DISK,&fcblist,sizeof(FCB)*FCB_ITEM_NUM,blocknum*BLOCK_SIZE,0);
    for(int i=0;i<FCB_ITEM_NUM;i++){
        if(fcblist[i].status==USED)
            num++;
    }
    return num;
}

//获得空的磁盘内存块
int getEmptyBlockId(){
    int flag=0;
    for(int i=0;i<FAT_ITEM_NUM;i++){
        if(FAT1[i].item==FREE&&flag==0){
            flag = i;
            break;
        }   
    }         
    if(flag==0)
        return -1;
    else
        return flag;
}

int getOpenNum(){
    int num=0;
    for(int i=0;i<MAX_FD_NUM;i++)
        if(uopenlist[i].topenfile==USED)
            num++;
    return num;
}

int getEmptyfd(){
    int fd=-1;
    for(int i=0;i<MAX_FD_NUM;i++)
        if((uopenlist[i].topenfile==FREE)&&(fd==-1)){
            fd=i;
            break;
        }  
    return fd;
}

//通过打开文件列表获取文件修饰符
int findfdByNameAndDir(char *filename,char *dirname){
    int fd=-1;
    for(int i=0;i<MAX_FD_NUM;i++)
        if(strcmp(filename,uopenlist[i].fcb.name)==0){
            if(strcmp(dirname,uopenlist[i].dir)==0){
                fd=i;
                break;
            }
        }  
    return fd;
}

int getNextBlocknum(int blocknum){
    return FAT1[blocknum].item;
}

blockchain* getBlockChain(int blocknum){
    int num = blocknum;
    blockchain *blc,*first;
    blc = get_node(struct blockchain);
    list_init(&(blc->link),blc);
    if(num==1||num==2||num==-1){//磁盘信息块或者是FCB块掠过
        first = get_node(struct blockchain);
        first->blocknum = blocknum;
        list_insert(&(blc->link),&(first->link),blc);
       return blc;
    }
    while(num!=-1){
        blockchain *temp;
        temp = get_node(struct blockchain);
        temp->blocknum = num;
        list_insert(&(blc->link),&(temp->link),blc);
        num = FAT1[num].item;
    }
    return blc;
}