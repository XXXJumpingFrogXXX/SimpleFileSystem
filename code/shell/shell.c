#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include"shell.h"
#include"../global/var.h"
#include"../tool/time.h"
#include"../api/api.h"
char ** split(char *string,char * delimiters,int *num){
    char ** result;
    char * arg;
    int a=1;
    result = (char **)malloc(sizeof(char *)*10);
    for(int i=0;i<10;i++)
        result[i] = (char *)malloc(sizeof(char)*10);
    arg = strtok(string,delimiters);
    result[0] = arg;
    while(arg!=NULL){
        arg = strtok(NULL,delimiters);
        result[a] = arg;
        a++;
        if(a==10){
            a++;
            break;
        }
    }
    if(arg!=NULL)
        *num=-1;
    else
        *num = a-1;
    return result;
}
char * trim(char *str){
    int len=0;
    char *temp = str;
    int start=0;
    if(temp==NULL||*temp=='\0')
        return str;
    while (*temp!='\0'&&isspace(*temp)){
        ++temp;
        ++start;
    }
    temp = str+strlen(str)-1;
    while(*temp!='\0'&&isspace(*temp))
        temp++;
    *(temp+1)='\0';
    return str+start;
}
char * header(){
    char *buff;
    buff = (char *)malloc(sizeof(char)*100);
    sprintf(buff,"\033[33m\033[01m%s\033[0m:\033[36m\033[01m%s\033[0m$ ",sysname,pwd);
    return buff;
}
char ** getInstruction(int *argc){
    char *buff;
    char **Ins;
    buff = (char *)malloc(sizeof(char)*100);
    Ins = (char **)malloc(sizeof(char *)*10);
    for(int i=0;i<10;i++)
        Ins[i] = (char *)malloc(sizeof(char)*10);
    printf("%s",header());
    fgets(buff,100,stdin);
    buff[strlen(buff)-1]='\0';
    buff = trim(buff);
    Ins = split(buff," ",argc);
    return Ins;
}
int doOperation(int argc,char ** argv){
    if(strcmp(argv[0],"help")==0){
        if(argc>1){
            printf("%s : too many arguments\n",argv[0]);
            return -1;
        }
        else{
            instruction_help();
            return 0;
        }  
    }

    if(strcmp(argv[0],"exit")==0){
        if(argc>1){
            printf("%s : too many arguments\n",argv[0]);
            return -1;
        }
        else{
            exit_system();
            return -2;
        }  
    }

    if(strcmp(argv[0],"pwd")==0){
        if(argc>1){
            printf("%s : too many arguments\n",argv[0]);
            return -1;
        }
        else{
            printf("%s\n",get_pwd());
            return 0;
        }  
    }

    if(strcmp(argv[0],"ls")==0){
        if(argc>1){
            printf("%s : too many arguments\n",argv[0]);
            return -1;
        }
        else{
            list_directory_contents();
            return 0;
        }  
    }

    if(strcmp(argv[0],"mkdir")==0){
        if(argc!=2){
            printf("usage %s [directory name]\n",argv[0]);
            return -1;
        }
        else{
            make_dir(argv[1]);
            return 0;
        }  
    }

    if(strcmp(argv[0],"cd")==0){
        if(argc!=2){
            printf("usage %s [directory name]\n",argv[0]);
            return -1;
        }
        else{
            cd_dir(argv[1]);
            return 0;
        }  
    }

    if(strcmp(argv[0],"touch")==0){
        if(argc!=2){
            printf("usage %s [file name]\n",argv[0]);
            return -1;
        }
        else{
            create_file(argv[1]);
            return 0;
        }  
    }

    if(strcmp(argv[0],"del")==0){
        if(argc!=2){
            printf("usage %s [file name]\n",argv[0]);
            return -1;
        }
        else{
            delete_file(argv[1]);
            return 0;
        }  
    }

    if(strcmp(argv[0],"rmdir")==0){
        if(argc!=2){
            printf("usage %s [directory name]\n",argv[0]);
            return -1;
        }
        else{
            remove_dir(argv[1]);
            return 0;
        }  
    }

    if(strcmp(argv[0],"open")==0){
        if(argc!=2){
            printf("usage %s [file name]\n",argv[0]);
            return -1;
        }
        else{
            open_file(argv[1]);
            return 0;
        }
    }

    if(strcmp(argv[0],"close")==0){
        if(argc!=2){
            printf("usage %s [fd num]\n",argv[0]);
            return -1;
        }
        else{
            int a;
            a = atoi(argv[1]);
            if(strcmp(argv[1],"0")&&a==0){
                printf("usage %s [fd num]\n",argv[0]);
                return -1;
            }
            close_file(a);
            return 0;
        }  
    }

    if(strcmp(argv[0],"write")==0){
        if(argc!=3){
            printf("usage %s [fd] [write method]\n",argv[0]);
            return -1;
        }
        else{
            int a1,len=0;
            char a2;
            a1 = atoi(argv[1]);
            a2 = argv[2][0];
            if((strcmp(argv[1],"0")&&a1==0)){
                printf("usage %s [fd] [write method]\n",argv[0]);
                return -1;
            }
            if(strlen(argv[1])!=1){
                printf("usage %s [fd] [write method]\n",argv[0]);
                return -1;
            }
            if(write_file(a1,&len,a2)==0){
                printf("Succeed! %d Bytes content has been written to fd %d.\n",len,a1);
                return 0;
            }
            return 0;
        }  
    }

    if(strcmp(argv[0],"read")==0){
        if(argc!=2){
            printf("usage %s [fd num]\n",argv[0]);
            return -1;
        }
        else{
            int a,len=0;
            a = atoi(argv[1]);
            if(strcmp(argv[1],"0")&&a==0){
                printf("usage %s [fd num]\n",argv[0]);
                return -1;
            }
            if(read_file(a,&len)==0)
                printf("read fd %d with %d bytes\n",a,len);
            return 0;
        }  
    }

    if(strcmp(argv[0],"time")==0){
        if(argc>1){
            printf("%s : too many arguments\n",argv[0]);
            return -1;
        }
        else{
            showCurrentTime();
            return 0;
        }  
    }
    printf("%s: command not found\n",argv[0]);
    return 0;
}
void go(){
    char buff[100];
    char **argv;
    int argc,flag;
    argv = (char **)malloc(sizeof(char *)*10);
    for(int i=0;i<10;i++)
        argv[i] = (char *)malloc(sizeof(char)*10);
    while(1){
        argv = getInstruction(&argc);
        flag = doOperation(argc,argv);
        if(flag==-2)
            break;
    }
    return;
}