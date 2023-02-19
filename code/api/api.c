#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include"api.h"
#include"../tool/disk.h"
#include"../tool/time.h"
#include"../global/var.h"
#include"../global/macro.h"

//指令帮助
void instruction_help(){
    printf("**********************指令帮助**********************\n");
    printf("%-10s - %s - %s\n","mkdir指令","创建文件夹","执行方式：mkdir [directory-name]");
    printf("%-10s - %s - %s\n","rmdir指令","删除文件夹","执行方式：rmdir [directory-name]");
    printf("%-10s - %s - %s\n","cd指令","改变工作目录","执行方式：cd [directory-name]");
    printf("%-10s - %s - %s\n","ls指令","列举出目录内容","执行方式：ls");
    printf("%-10s - %s - %s\n","touch指令","创建文件","执行方式：touch [file-name]");
    printf("%-10s - %s - %s\n","del指令","删除文件","执行方式：del [file-name]");    
    printf("%-10s - %s - %s\n","open指令","打开文件","执行方式：open [file-name]");
    printf("%-10s - %s - %s\n","write指令","写入文件","执行方式：write [fd num] [write method]");
    printf("--------write method可输入w/a/c | w: 截断写 a :追加写 c :覆盖写\n");
    printf("--------fd num --- 文件描述符序号 | open时可以看见对应的文件fd num\n");
    printf("!!!!!!!!!!!!!想要结束写操作 输入end之后敲击回车!!!!!!!!!\n");
    printf("%-10s - %s - %s\n","read指令","读取文件","执行方式：read [fd num]");
    printf("%-10s - %s - %s\n","close指令","关闭文件","执行方式：close [fdnum]");
    printf("%-10s - %s - %s\n","pwd指令","获取当前所在工作目录路径","执行方式：pwd");
    printf("%-10s - %s - %s\n","time指令","打印当前时间","执行方式：time");
    printf("%-10s - %s - %s\n","exit指令","退出系统","执行方式：exit");
}

//文件系统格式化
void format(){
//引导块
    strcpy(block0.identifier,"DISK");
    strcpy(block0.info,"creator: 王麒翔 blocksize:1024B blocknum:1024 Disksize:1MB");
    block0.root = ROOT_FCB_LOCATION;
    block0.startblock = DATA_INIT_BLOCK;
    block0.rootFCB = ROOT_FCB_LOCATION;
    fseek(DISK,0,SEEK_SET);
    fwrite(&block0,sizeof(BLOCK0),1,DISK);
//FAT1+FAT2
    FATitem *fat;
    fat = (FATitem *)malloc(sizeof(FATitem)*BLOCK_NUMS*2);
    memset(fat,FREE,sizeof(FATitem)); // prob
    fseek(DISK,1*BLOCK_SIZE,SEEK_SET); // prob
    fwrite(fat,sizeof(FATitem),BLOCK_NUMS*2,DISK);
    free(fat);
//root(/)
    struct tm *t=getTimeStruct();
    FCB rootFCB;
    strcpy(rootFCB.name,"/");
    rootFCB.type = 1;       //类型-目录
    rootFCB.status = USED;        //已使用
    rootFCB.time = getTime(t);
    rootFCB.date = getDate(t);
    rootFCB.base = 6;       //起始盘块号
    rootFCB.length = 1;     //长度
    FAT1[5].item=FCB_BLOCK; 
    FAT2[5].item=FCB_BLOCK; 
    addFCB(rootFCB,5);
    initFCBBlock(6,6);
//重写FAT表
    for(int i=0;i<5;i++)
        FAT1[i].item=USED;
    rewriteFAT();
}

//初始化文件系统
void init_system(){
    int fd;
//以创建新文件的方式打开
    if((fd=open(sysname,O_CREAT|O_EXCL,S_IRWXU))<0){
        if(errno==EEXIST){
        //直接打开这个文件
            if((fd = open(sysname,O_RDWR,S_IRWXU))<0){
                printf("open:%d  %s\n",errno,strerror(errno));//打开失败
                exit(-1);
            }
        }
        else//其他原因
            exit(-1);
    }else{
        //如果采取新建一个文件，那么新建一个磁盘文件并分配空间
        close(fd);
        createDisk();
        if((fd = open(sysname,O_RDWR,S_IRWXU))<0){
            printf("open:%d  %s\n",errno,strerror(errno));
            exit(-1);
        }
    }
//变成标准IO流
    if((DISK = fdopen(fd,"r+"))==NULL){
        printf("fdopen:%d  %s\n",errno,strerror(errno));
        exit(-1);
    }
    char buff[10];
    if(fread(&block0,sizeof(BLOCK0),1,DISK)==1)

//判断是否格式化            
    if(strcmp(block0.identifier,"DISK")!=0){
        printf("Format the disk.\n");
        format();
    }
//进行其他初始化操作
    strcpy(pwd,"/");//设置当前目录
    getFCB(&presentFCB,5,0);//将根目录设置成当前内存中的FCB
    getFAT(FAT1,FAT1_LOCATON);//加载FAT1
    getFAT(FAT2,FAT2_LOCATON);//加载FAT2
//初始化文件打开表
    for(int i=0;i<MAX_FD_NUM;i++)
        uopenlist[i].topenfile=FREE;//全部设置成0
    return;
}

//返回当前所在工作目录的全路径
char *get_pwd(){
    return pwd;
}

//创建目录
int make_dir(char *dirname){
//判断文件名长度
    if(strlen(dirname)>FILE_NAME_LEN){
        printf("mkdir: cannot create directory ‘%s’: directory name must less than %d bytes\n",dirname,FILE_NAME_LEN);
        return -1;
    }
//检查是否重名
    if(findFCBInBlockByName(dirname,presentFCB.base)>0){
        printf("mkdir: cannot create directory ‘%s’: File exists\n",dirname);
        return -1;
    } 
//获得当前目录表中空余位置，以便存放FCB
    int offset = getEmptyFCBOffset(presentFCB.base);
    if(offset<0){
        printf("mkdir:no empty space to make directory\n");
        return -1;
    }
//获得空的盘块以便存储新的目录表
    int blocknum = getEmptyBlockId();
    FCB fcb;
    if(blocknum<0){
        printf("mkdir:no empty block\n");
        return -1;
    }
//构造FCB
    struct tm *t=getTimeStruct();
    strcpy(fcb.name,dirname);
    fcb.type=1;
    fcb.status=USED;
    fcb.time=getTime(t);
    fcb.date=getDate(t);
    fcb.base=blocknum;
    fcb.length=1;
    initFCBBlock(blocknum,presentFCB.base);
    addFCB(fcb,presentFCB.base);//在当前目录表加入这个目录项
    rewriteFAT();
}

//删除目录
int remove_dir(char *dirname){
//检查是否有这个目录
    int offset;
    FCB fcb;//将要删除的FCB
    if((offset=findFCBInBlockByName(dirname,presentFCB.base))<0){
        printf("rmdir: cannot remove '%s': No such file or directory\n",dirname);
        return -1;
    }
    else{
    // 清空这个FCB指向的盘块
        getFCB(&fcb,presentFCB.base,offset);
    //判断是否为目录类型
        if(fcb.type!=1){
            printf("rmdir: cannot remove '%s': Is a file, please use rm\n",dirname);
            return -1;
        }
    //判断是否删除的是 .或 ..目录
        if(strcmp(fcb.name,".")==0||strcmp(fcb.name,"..")==0){
            printf("rmdir: refusing to remove '.' or '..'\n");
            return -1;
        }
    //判断这个目录是否为空
        for(int i=0;i<FCB_ITEM_NUM;i++){
            FCB f;
            getFCB(&f,fcb.base,i);
            if(f.status==USED){
                if(strcmp(f.name,".")!=0&&strcmp(f.name,"..")!=0){
                    printf("rmdir: cannot remove '%s': %s not empty\n",dirname,dirname);
                    return -1;
                }
            }
        }
    //修改FAT
        FAT1[fcb.base].item=FREE;
        FAT2[fcb.base].item=FREE;
        rewriteFAT();
    //删除这个FCB
        removeFCB(presentFCB.base,offset);
        return 0;
    }
}

//列出内容
void list_directory_contents(){
    int blocknum = presentFCB.base;
    lslink *FCBlisthead,*temp;
    FCBList FL,*Fnode;
    FCBlisthead = &(FL.link);
    unsigned short date;
    unsigned short time;
    printf("当前目录为： %s\n",pwd);
    printf("%-12s        %-15s           %-20s\n","文件名称","文件类型","创建时间");
    getFCBList(blocknum,FL,FCBlisthead);
    list_for_each(temp,FCBlisthead){
        Fnode = list_entry(temp,FCBList,link);
        date = Fnode->fcb_entry.date;
        time = Fnode->fcb_entry.time;
        printf("%-12s   %-15s  %4d/%02d/%02d %02d:%02d:%02d\n",Fnode->fcb_entry.name,
        type[Fnode->fcb_entry.type],getYear(date),getMonth(date),getDay(date),
        getHour(date,time),getMinute(time),getSecond(time));
    }
}

//进入目录
int cd_dir(char *dirname){
    int offset;
    if((offset=findFCBInBlockByName(dirname,presentFCB.base))<0){
        printf("cd: %s: No such file or directory\n",dirname);
        return -1;
    }else{
        FCB fcb;
        getFCB(&fcb,presentFCB.base,offset);
        if(fcb.type==0){
            printf("cd: %s: Not a directory\n",dirname);
            return -1;
        }
        presentFCB=fcb;//修改当前fcb值
        if(strcmp(dirname,".")==0)//当前目录
            ;
        else if(strcmp(dirname,"..")==0){//上一级目录
            if(strcmp(pwd,"/")!=0){//不是根目录情况
                char *a = strchr(pwd,'/');//从左往右第一次出现/的位置
                char *b = strrchr(pwd,'/');//从右往左第一次出现/的位置
                if(a!=b)//判断是否只有一个/字符 不相等则有多个
                    *b='\0';
                else
                    *(b+1)='\0';
            }
        }
        else{//下一级目录
            if(strcmp(pwd,"/")!=0)
                strcat(pwd,"/");
            strcat(pwd,dirname);
        }
        return 0;
    }    
}

//创建文件
int create_file(char *filename){
    //判断文件名长度
    if(strlen(filename)>FILE_NAME_LEN){
        printf("create: cannot create file ‘%s’: file name must less than %d bytes\n",filename,FILE_NAME_LEN);
        return -1;
    }
    int offset = findFCBInBlockByName(filename,presentFCB.base);
    if(offset>0){//判断是否已经存在此文件
        printf("create: cannot create file ‘%s’: File exists\n",filename);
        return -1;
    }else{
        offset = getEmptyFCBOffset(presentFCB.base);//判断FCB块是否有剩余空间加入FCB
        if(offset<0){
            printf("create: cannot create file ‘%s’: %s Lack of space\n",pwd,filename);
            return -1;
        }else{
            int blocknum = getEmptyBlockId();
            if(blocknum<0){
                printf("create: cannot create file ‘%s’: %s Lack of space\n",sysname,filename);
                return -1;
            }else{
            //构建FCB
                struct tm *t=getTimeStruct();
                FCB fcb;
                strcpy(fcb.name,filename);
                fcb.type=0;
                fcb.status=USED;
                fcb.time=getTime(t);
                fcb.date=getDate(t);
                fcb.base=blocknum;
                fcb.length=1;
                addFCB(fcb,presentFCB.base);
            //重写FAT
                FAT1[blocknum].item=END_OF_FILE;
                FAT2[blocknum].item=END_OF_FILE;
                rewriteFAT();
                return 0;
            }
        }
    }
    return 0;
}

//删除文件
int delete_file(char *filename){
    int offset = findFCBInBlockByName(filename,presentFCB.base);
    if(offset<0){//判断是否已经存在此文件
        printf("rm: cannot remove '%s': No such file\n",filename);
        return -1;
    }
    else{
        FCB fcb;
        getFCB(&fcb,presentFCB.base,offset);
    //判断文件类型
        if(fcb.type==1){
            printf("rm: cannot remove '%s': Is a directory\n",filename);
            return -1;
        }
    //重写FAT
        blockchain *blc;
        lslink *temp;
        blc = getBlockChain(fcb.base);
        list_for_each(temp,&(blc->link)){
            blockchain *b = list_entry(temp,struct blockchain,link);
            FAT1[b->blocknum].item=FREE;
            FAT2[b->blocknum].item=FREE;
        }
        FAT1[fcb.base].item=FREE;
        FAT2[fcb.base].item=FREE;
        rewriteFAT();
    //删除FCB
        removeFCB(presentFCB.base,offset);
        int fd = findfdByNameAndDir(filename,pwd);
        if(fd>=0&&uopenlist[fd].topenfile==USED)
            uopenlist[fd].topenfile=FREE;
        return 0;
    }
}

//打开文件
int open_file(char *filename){
    int fd; //文件修饰符
    fd = findfdByNameAndDir(filename,pwd);
    if(fd>=0&&uopenlist[fd].topenfile==USED){//判断是否已经打开
        printf("open: cannot open file ‘%s’: %s is already open\n",filename,filename);
        return -1;
    }
    if((fd=getEmptyfd())<0){//查看是否有空的fd
        printf("open: cannot open file ‘%s’: Lack of empty fd\n",filename);
        return -1;
    }
    else{
        int offset = findFCBInBlockByName(filename,presentFCB.base);//获得fcb位置
        if(offset<0){
            printf("open: %s: No such file or directory\n",filename);
            return -1;
        }else{
        //构造打开表项
            FCB fcb;
            getFCB(&fcb,presentFCB.base,offset);
            uopenlist[fd].fcb = fcb;
            strcpy(uopenlist[fd].dir,pwd);
            uopenlist[fd].count = BLOCK_SIZE*fcb.base+0;
            uopenlist[fd].fcbstate = 0;
            uopenlist[fd].topenfile = USED;
            uopenlist[fd].blocknum = presentFCB.base;
            uopenlist[fd].offset_in_block = findFCBInBlockByName(filename,presentFCB.base);
            printf("filename:%s fdnum:%d\n",filename,fd);
            return 0;
        }
    }
}

//关闭文件
int close_file(int fd){
    if(fd>=MAX_FD_NUM||fd<0){
        printf("close: invalid fd\n");
        return -1;
    }else{
        if(uopenlist[fd].topenfile==FREE){//判断是否已经关闭
            printf("close: cannot close fd ‘%d’: fd %d is already close\n",fd,fd);
            return -1;
        }
        if(uopenlist[fd].fcbstate==1)
            changeFCB(uopenlist[fd].fcb,uopenlist[fd].blocknum,uopenlist[fd].offset_in_block);
        uopenlist[fd].topenfile=FREE;//清空文件打开表项
        return 0;
    }
}

//写入文件
int write_file(int fd,int *sumlen,char wstyle){
    //sumlen记录写了多少个字符
    if(fd>=MAX_FD_NUM||fd<0){//判断fd合法性
        printf("close: invalid fd\n");
        return -1;
    }
    else{
        if(uopenlist[fd].topenfile==FREE){//判断是否已经关闭
            printf("write: cannot write to fd ‘%d’: fd %d is already close\n",fd,fd);
            return -1;
        }else{
            if(uopenlist[fd].fcb.type==1){//判断如果是目录
                printf("write: cannot write to fd ‘%d’: fd %d is a directory\n",fd,fd);
                return -1;
            }
            char str[BLOCK_SIZE],buff[BLOCK_SIZE]; //str单次输入的字符串 buffer汇总输入的字符串
            int blocknum,nextblocknum; //blocknum当前数据块号 nextblocknum下一个数据块号（如果有的话）
            int len,bloffset; //len-一次读取的长度 bloffset-文件指针块内偏移量
            *sumlen=0; //sumlen-总长(所有输入长度之和)
            memset(str,0,BLOCK_SIZE);
            memset(buff,0,BLOCK_SIZE);
            //截断写
            if(wstyle=='w'){
                bloffset = 0;
                blocknum = uopenlist[fd].fcb.base;
            //做截断处理
                if(FAT1[blocknum].item!=END_OF_FILE){
                    blockchain *blc = getBlockChain(blocknum);
                    lslink *temp;
                    list_for_each(temp,&(blc->link)){
                        blockchain *b = list_entry(temp,struct blockchain,link); //找到一个块
                    //清空对应FAT块
                        FAT1[b->blocknum].item=FREE;
                        FAT2[b->blocknum].item=FREE;
                        uopenlist[fd].fcb.length=0;
                    }
                }
            //循环读取直到EOF 每次最多读取一个盘块大小的内容 多余部分留在缓冲区作为下次读取
                while(fgets(str,BLOCK_SIZE,stdin)!=NULL){
                    if(strcmp(str,"end\n") == 0){
                        break;
                    }               
                    len = strlen(str);//记录实际读取到的长度
                    //printf("len %d\n",len);
                    if(bloffset+len<BLOCK_SIZE){
                //文件指针块内偏移量和将要输入的长度之和小于一个盘块--不停地输入到缓冲区中
                        *sumlen+=len;
                        bloffset+=len;
                        strcat(buff,str);
                    }else{
                //长度大于一个盘块
                        int lastlen=BLOCK_SIZE-bloffset-1;//要留出一位给\0作为结尾标志
                        int leavelen=len-lastlen;//计算剩下的长度 输入留下的长度
                        strncat(buff,str,lastlen);//填满上一个块
                        writeToDisk(DISK,buff,BLOCK_SIZE,blocknum*BLOCK_SIZE,0);//先把之前整个盘块的内容存放起来
                        memset(buff,'\0',BLOCK_SIZE);//重新初始化buff 清空原有数据 准备下一次输入
                    //修改总长度和块内指针位置
                        *sumlen+=len;
                        bloffset = leavelen;
                    //将剩下部分输入缓冲区
                        strcat(buff,str+lastlen);
                    //获得一个新的盘块
                        nextblocknum = getEmptyBlockId();
                    //判断空间是否充足  
                        if(nextblocknum<0){
                            printf("write: cannot write to fd ‘%d’: lack of space\n",fd);
                            return -1;
                        }
                    //标记使用状态
                        FAT1[nextblocknum].item=END_OF_FILE;
                        FAT2[nextblocknum].item=END_OF_FILE;
                    //修改FAT
                        FAT1[blocknum].item=nextblocknum;
                        FAT2[blocknum].item=nextblocknum;
                        blocknum = nextblocknum;//修改当前块号
                    }
                }
                writeToDisk(DISK,buff,BLOCK_SIZE,blocknum*BLOCK_SIZE,0);
                uopenlist[fd].count=BLOCK_SIZE*blocknum+bloffset;
                uopenlist[fd].fcb.length = *sumlen;
                //修改对应FCB
                changeFCB(uopenlist[fd].fcb,uopenlist[fd].blocknum,uopenlist[fd].offset_in_block);
                //设置文件结尾
                FAT1[blocknum].item=END_OF_FILE;
                FAT2[blocknum].item=END_OF_FILE;
                rewriteFAT();
                return 0;
            }
            //追加写
            else if(wstyle=='a'){
                int currentlen=uopenlist[fd].fcb.length;//计算写前长度
                //计算块号
                blocknum=uopenlist[fd].fcb.base;
                while(FAT1[blocknum].item!=END_OF_FILE){
                    blocknum=FAT1[blocknum].item;
                }
                int oldblocknum = blocknum;//记录写前最后一个块号
                //计算块内偏移量
                bloffset=currentlen%BLOCK_SIZE;
                //循环读取直到EOF 每次最多读取一个盘块大小的内容 多余部分留在缓冲区作为下次读取
                while(fgets(str,BLOCK_SIZE,stdin)!=NULL){
                    if(strcmp(str,"end\n") == 0){
                        break;
                    }
                    len = strlen(str);//记录实际读取到的长度
                    if(bloffset+len<BLOCK_SIZE){
                    //文件指针块内偏移量和将要输入的长度之和小于一个盘块--不停地输入到缓冲区中
                        *sumlen+=len;
                        bloffset+=len;
                        strcat(buff,str);
                    }else{
                    //长度大于一个盘块
                        int lastlen=BLOCK_SIZE-bloffset-1;//要留出一位给\0作为结尾标志
                        int leavelen=len-lastlen;//计算剩下的长度
                        strncat(buff,str,lastlen);//填满上一个块
                        if(blocknum==oldblocknum)//和w的区别 这里要分类处理 讨论是不是要填满原有的块
                            writeToDisk(DISK,buff,strlen(buff),blocknum*BLOCK_SIZE,BLOCK_SIZE-strlen(buff)-1);//填满上一次没有写满盘块
                        else
                            writeToDisk(DISK,buff,BLOCK_SIZE,blocknum*BLOCK_SIZE,0);//往一个全新的盘块写
                        memset(buff,'\0',BLOCK_SIZE);//重新初始化buff 清空原有数据 准备下一次输入
                    //修改总长度和块内指针位置
                        *sumlen+=len;
                        bloffset = leavelen;
                    //将剩下部分输入缓冲区
                        strcat(buff,str+lastlen);
                    //获得一个新的盘块
                        nextblocknum = getEmptyBlockId();
                    //判断空间是否充足  
                        if(nextblocknum<0){
                            printf("write: cannot write to fd ‘%d’: lack of space\n",fd);
                            return -1;
                        }
                        FAT1[nextblocknum].item=END_OF_FILE;
                        FAT2[nextblocknum].item=END_OF_FILE;
                    //修改FAT
                        FAT1[blocknum].item=nextblocknum;
                        FAT2[blocknum].item=nextblocknum;
                        blocknum = nextblocknum;//修改当前块号
                    }
                }
                if(blocknum==oldblocknum)//和w的区别 这里要分类处理 讨论是不是要填满原有的第一个块
                    writeToDisk(DISK,buff,strlen(buff),blocknum*BLOCK_SIZE,bloffset-*sumlen);//填满上一次没有写满盘块
                else
                    writeToDisk(DISK,buff,BLOCK_SIZE,blocknum*BLOCK_SIZE,0);//往一个全新的盘块写
                //writeToDisk(DISK,buff,strlen(buff),blocknum*BLOCK_SIZE,bloffset-strlen(buff));
                uopenlist[fd].count=BLOCK_SIZE*blocknum+bloffset;
                uopenlist[fd].fcb.length = currentlen+*sumlen;
            //修改对应FCB
                changeFCB(uopenlist[fd].fcb,uopenlist[fd].blocknum,uopenlist[fd].offset_in_block);
            //文件结尾
                FAT1[blocknum].item=END_OF_FILE;
                FAT2[blocknum].item=END_OF_FILE;
                rewriteFAT();
                return 0; 
            }
            //覆盖写
            else if(wstyle=='c'){
                int currentlen=uopenlist[fd].fcb.length;//计算当前(写前)长度
                bloffset=uopenlist[fd].count%BLOCK_SIZE;//计算块内偏移量
                blocknum=uopenlist[fd].count/BLOCK_SIZE;//计算块号
                int oldblocknum = blocknum;//记录写前最后一个块号
                //循环读取直到EOF 每次最多读取一个盘块大小的内容 多余部分留在缓冲区作为下次读取
                while(fgets(str,BLOCK_SIZE,stdin)!=NULL){
                    if(strcmp(str,"end\n") == 0){
                        break;
                    }                    
                    len = strlen(str);//记录实际读取到的长度
                    if(bloffset+len<BLOCK_SIZE){
                    //文件指针块内偏移量和将要输入的长度之和小于一个盘块--不停地输入到缓冲区中
                        *sumlen+=len;
                        bloffset+=len;
                        strcat(buff,str);
                    }else{
                    //长度大于一个盘块
                        int lastlen=BLOCK_SIZE-bloffset-1;//要留出一位给\0作为结尾标志
                        int leavelen=len-lastlen;//计算剩下的长度
                        strncat(buff,str,lastlen);//填满上一个块
                        if(blocknum==oldblocknum)//和w的区别 这里要分类处理 讨论是不是要填满原有的第一个块
                            writeToDisk(DISK,buff,strlen(buff),blocknum*BLOCK_SIZE,bloffset);//填满上一次没有写满盘块
                        else
                            writeToDisk(DISK,buff,BLOCK_SIZE,blocknum*BLOCK_SIZE,0);//往一个全新的盘块写
                        memset(buff,'\0',BLOCK_SIZE);//重新初始化buff 清空原有数据 准备下一次输入
                    //修改总长度和块内指针位置
                        *sumlen+=len;
                        bloffset = leavelen;
                    //将剩下部分输入缓冲区
                        strcat(buff,str+lastlen);
                    //获得一个新的盘块
                        nextblocknum = getEmptyBlockId();
                    //判断空间是否充足  
                        if(nextblocknum<0){
                            printf("write: cannot write to fd ‘%d’: lack of space\n",fd);
                            return -1;
                        }
                        FAT1[nextblocknum].item=END_OF_FILE;
                        FAT2[nextblocknum].item=END_OF_FILE;
                    //修改FAT
                        FAT1[blocknum].item=nextblocknum;
                        FAT2[blocknum].item=nextblocknum;
                        blocknum = nextblocknum;//修改当前块号
                    }
                }
                if(blocknum==oldblocknum)//和w的区别 这里要分类处理 讨论是不是要填满原有的第一个块
                    writeToDisk(DISK,buff,strlen(buff),blocknum*BLOCK_SIZE,bloffset-strlen(buff));//填满上一次没有写满盘块
                else
                    writeToDisk(DISK,buff,BLOCK_SIZE,blocknum*BLOCK_SIZE,0);//往一个全新的盘块写
                uopenlist[fd].count=BLOCK_SIZE*blocknum+bloffset;
                uopenlist[fd].fcb.length = currentlen+*sumlen;
            //修改对应FCB
                changeFCB(uopenlist[fd].fcb,uopenlist[fd].blocknum,uopenlist[fd].offset_in_block);
            //文件结尾
                FAT1[blocknum].item=END_OF_FILE;
                FAT2[blocknum].item=END_OF_FILE;
                rewriteFAT();
                return 0;
            }else{
                printf("write: invalid write type ‘%c’\n",wstyle);
                return -1;
            }
        } 
    }
}

//读取文件
int read_file(int fd,int *sumlen){
    if(fd>=MAX_FD_NUM||fd<0){
        printf("read: invalid fd\n");
        return -1;
    }
    else{
        //判断是否已经关闭
        if(uopenlist[fd].topenfile==FREE){
            printf("read: cannot read to fd ‘%d’: fd %d is already close\n",fd,fd);
            return -1;
        }
        else{
            //如果所读文件为目录类型
            if(uopenlist[fd].fcb.type==1){
                printf("read: cannot read to fd ‘%d’: fd %d is a directory\n",fd,fd);
                return -1;
            }
            char buff[BLOCK_SIZE];
            int blocknum = uopenlist[fd].fcb.base;
            while(blocknum!=END_OF_FILE){
                readFromDisk(DISK,buff,BLOCK_SIZE,blocknum*BLOCK_SIZE,0);
                *sumlen += strlen(buff);
                fputs(buff,stdout);
                blocknum=FAT1[blocknum].item;
            }
            return 0;
        }
    }
}

//退出文件系统
void exit_system(){
    fclose(DISK);
}
