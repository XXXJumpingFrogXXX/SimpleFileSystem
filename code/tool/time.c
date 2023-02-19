#include"time.h"
#include<stdio.h>
struct tm* getTimeStruct(){
    time_t t = time(NULL);
    return localtime(&t);
}
unsigned short getDate(struct tm* t){
    int year = t->tm_year-70;
    int mon = t->tm_mon;
    int day = t->tm_mday;
    unsigned short result=0;
    if(t->tm_hour>12)
        result = 1<<15;
    result += year*12*31+mon*31+day;
    return result;
}

unsigned short getTime(struct tm* t){
    unsigned short hour = t->tm_hour;
    unsigned short min = t->tm_min;
    unsigned short sec = t->tm_sec;
    if(hour>12)
        hour-=12;
    return hour*3600+min*60+sec;
}

unsigned short getHour(unsigned short date,unsigned short time){
    if(date>>15==1)
        return 12 + time/3600;
    else
        return time/3600;
}

unsigned short getMinute(unsigned short time){
    return (time%3600)/60;
}

unsigned short getSecond(unsigned short time){
    return (time%3600)%60;
}

unsigned short getYear(unsigned short time){
    time = time & 32767;
    return 1970+time/372;
}

unsigned short getMonth(unsigned short time){
    time = time & 32767;
    return (time%372)/31+1;
}

unsigned short getDay(unsigned short time){
    time = time & 32767;
    return (time%372)%31;
}

void showCurrentTime(){
    struct tm* ts = getTimeStruct();
    unsigned short date = getDate(ts);
    unsigned short time = getTime(ts);
    printf("%4d/%02d/%02d %02d:%02d:%02d\n",
    getYear(date),getMonth(date),getDay(date),
    getHour(date,time),getMinute(time),getSecond(time));
}