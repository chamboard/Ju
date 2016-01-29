#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>			// RTC functions + ..
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <math.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sched.h>


int main (int argc,char *argv[ ]) {
clock_t t;
	struct timespec fps_start;clock_gettime(CLOCK_MONOTONIC,&fps_start);
	struct timespec fps_end;
uint64_t dbg_time_max=0xffffffffffffffff;
	uint64_t dbg_time=0;




//system("./chanram4 -l");
time_t secondes;



#include <stdio.h>
#include <math.h>


   /*char str[80];

   sprintf(str, "Value of Pi = %f", M_PI);
   puts(str);
   
   return(0);*/



    struct tm instant;

    time(&secondes);

    instant=*localtime(&secondes);

    printf("%d/%d/%d ; %d ; %d:%d:%d\n", instant.tm_mday, instant.tm_mon+1,instant.tm_year-100,instant.tm_wday, instant.tm_hour, instant.tm_min, instant.tm_sec);
int sec,min,hour,wday,mday,mon,year;
sec=((instant.tm_sec/10)*16+instant.tm_sec%10)+1;
min=(instant.tm_min/10)*16+instant.tm_min%10;
hour=(instant.tm_hour/10)*16+instant.tm_hour%10;
wday=(instant.tm_wday/10)*16+instant.tm_wday%10;
year=(instant.tm_year-100)/10*16+(instant.tm_year-100)%10;
mon=(instant.tm_mon+1)/10*16+(instant.tm_mon+1)%10;
mday=instant.tm_mday/10*16+instant.tm_mday%10;

/*printf("sec : 0x%2x \n",sec);
printf("min : 0x%2x \n",min);
printf("hour : 0x%2x \n",hour);
printf("wday : 0x%2x \n",wday);
printf("mday : 0x%2x \n",mday);
printf("mon : 0x%2x \n",mon);
printf("year : 0x%2x \n",year);*/

char cmd[256]="\0";

sprintf(cmd,"./chanram4 -a 7 -p 0x140 -w1 -v 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x ",sec,min,hour,wday,mday,mon,year);

//printf("cmd : %s\n",cmd);
//fflush(stdout);
system("./chanram4 -a 7 -e 1"); 
system(cmd);
system("./chanram4 -a 7 -e 2");


clock_gettime(CLOCK_MONOTONIC,&fps_end);
printf("DEBUG TIME START : %lu \n\n",fps_start.tv_nsec);
//printf("time stop : %lu \n",fps_end.tv_nsec); 
if(fps_end.tv_nsec>fps_start.tv_nsec) dbg_time=(uint64_t) fps_end.tv_nsec- (uint64_t)fps_start.tv_nsec;
else dbg_time=dbg_time_max-(uint64_t)fps_start.tv_nsec+(uint64_t) fps_end.tv_nsec;
printf("DEBUG TIME END : %lu \n\n",(unsigned long)dbg_time);/**/
return 0;
}
