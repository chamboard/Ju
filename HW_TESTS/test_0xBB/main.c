#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>

#include <CHANTILLY_BP.h>
#include "/home/pi/code_004/common/CHAN/coredef_0xBB_139_020.h"
 
//#include <CHANTILLY_BP.h>
//#include "/home/pi/code_004/common/CHAN/daemon_defs.h"
//#include "/home/pi/code_004/common/CHAN/daemon_utils.h"
//#include "/home/pi/code_004/common/CHAN/CHAN_FUNCTIONS_BP.h"

/*
#include "local_config.h"
//
#include "/home/pi/code_004/common/CHAN/coredef_0xBB_139_020.h"
#include "/home/pi/code_004/common/CHAN/debug_utils.h"
#include "/home/pi/code_004/common/CHAN/core_utils.h"
#include "/home/pi/code_004/common/CHAN/corefunc_0xBB.h"*/

//#include <CHANTILLY_BP.h>

#define _BUS_BOARD      0x07
#define _THIS_APP_ID 			0xFF
 
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

uint8_t CORE_ID[7]={0};
uint8_t CORE_NAME[7][20];
uint8_t CORE_NUMBER[7]={0};
uint8_t myCORES[7][4];

int kbhit(void)
{
	 int i;
	 ioctl(0, FIONREAD, &i);
	 return i; 															// Returns a count of characters available to read (from stdin=fd/0)
}

void getch_start(void)	{  system("stty raw -echo"); }

void getch_stop(void) { system("stty cooked echo"); }

unsigned char getch_try(void)               							// Gets the last character typed without stopping program.
{
	int c=0;
	while (kbhit()) { c=getchar(); }
	return (unsigned char) c;											// One char from queue only.
}	

uint8_t testSelectCore(uint8_t i) {
	char c=0;
	if (i==0) {
		printf(ANSI_COLOR_YELLOW"** SELECT CORES TEST **\n"ANSI_COLOR_RESET);
		i=1;
	}
	CHAN_select(i);
	printf(ANSI_COLOR_YELLOW"Core %d"ANSI_COLOR_RESET" is selected, verify voltage is 0V on "ANSI_COLOR_YELLOW"pin %d"ANSI_COLOR_RESET". y/n ? \n",i,i);
	while (c!='n' && c!='y') {
		getch_start();																//
		c=getch_try();																//
		getch_stop();	
	}
	if (c=='y') {
		printf(ANSI_COLOR_GREEN"SELECT CORE %d TEST OK\n"ANSI_COLOR_RESET,i);
		if (i<7) return testSelectCore(++i);
		else {
			return 1;
		}
	}
	else {
		printf(ANSI_COLOR_RED"SELECT CORE %d TEST KO\n"ANSI_COLOR_RESET,i);
		return 0;
	}
}
	
uint8_t testRTC(uint32_t* cErrors) {
	uint8_t myRXBuffer[256];
	uint8_t myTXBuffer[256]={0};
	struct tm tm_chan;
	struct tm tm_now;
	time_t now;
	char s[sizeof "JJ/MM/AAAA HH:MM:SS"];
	//time_t now = time(NULL);										/*read current time*/
	//struct tm tm_now = *localtime(&now);							/*make local time*/
	//char s_now[sizeof "JJ/MM/AAAA HH:MM:SS"];						/*make a string JJ/MM/AAAA HH:MM:SS */
	//strftime (s_now, sizeof s_now, "%d/%m/%Y %H:%M:%S", &tm_now);
	printf(ANSI_COLOR_YELLOW"** RTC TEST **\n"ANSI_COLOR_RESET);
	printf("Reset Chantilly datetime : \t");
	*cErrors+=CHAN_addr(_BUS_BOARD,_BB_RTC_DATA);
	*cErrors+=CHAN_setBytes(_BUS_BOARD,10,myTXBuffer);
	*cErrors+=CHAN_command(_BUS_BOARD,0x02);
	*cErrors+=CHAN_command(_BUS_BOARD,0x01);
	*cErrors+=CHAN_addr(_BUS_BOARD,_BB_RTC_DATA);
	*cErrors+=CHAN_getBytes(_BUS_BOARD,7,&myRXBuffer[0]);
	tm_chan.tm_sec=myRXBuffer[0];
	tm_chan.tm_min=myRXBuffer[1];
	tm_chan.tm_hour=myRXBuffer[2];
	tm_chan.tm_mday=myRXBuffer[4];
	tm_chan.tm_mon=myRXBuffer[5]-1;
	tm_chan.tm_year=myRXBuffer[6]+100;
	strftime (s, sizeof s, "%d/%m/%Y %H:%M:%S", &tm_chan);
	printf (ANSI_COLOR_YELLOW"Chantilly datetime : \t'%s'\n"ANSI_COLOR_RESET, s);
	printf("Reading System datetime : \t");
	now = time(NULL);															/*read current time*/
	tm_now = *localtime(&now);													/*make local time*/
	strftime (s, sizeof s, "%d/%m/%Y %H:%M:%S", &tm_now);
	printf (ANSI_COLOR_YELLOW"System datetime : \t'%s'\n"ANSI_COLOR_RESET, s);	/*display result*/
	fflush(stdout);
    printf("Setting Chantilly datetime : \t");
	myTXBuffer[1]=tm_now.tm_min;
	myTXBuffer[2]=tm_now.tm_hour;
	myTXBuffer[4]=tm_now.tm_mday;
	//myTXBuffer[3]=tm_now.tm_wday;
	myTXBuffer[5]=tm_now.tm_mon+1;
	myTXBuffer[6]=tm_now.tm_year-100;
	myTXBuffer[0]=tm_now.tm_sec;
	*cErrors+=CHAN_addr(_BUS_BOARD,_BB_RTC_DATA);
	*cErrors+=CHAN_setBytes(_BUS_BOARD,7,myTXBuffer);
	*cErrors+=CHAN_command(_BUS_BOARD,0x02);
	*cErrors+=CHAN_command(_BUS_BOARD,0x01);
	*cErrors+=CHAN_addr(_BUS_BOARD,_BB_RTC_DATA);
	*cErrors+=CHAN_getBytes(_BUS_BOARD,7,&myRXBuffer[0]);
	tm_chan.tm_sec=myRXBuffer[0];
	tm_chan.tm_min=myRXBuffer[1];
	tm_chan.tm_hour=myRXBuffer[2];
	tm_chan.tm_mday=myRXBuffer[4];
	tm_chan.tm_mon=myRXBuffer[5]-1;
	tm_chan.tm_year=myRXBuffer[6]+100;
	strftime (s, sizeof s, "%d/%m/%Y %H:%M:%S", &tm_chan);
	printf (ANSI_COLOR_YELLOW"Chantilly datetime : \t'%s'\n"ANSI_COLOR_RESET, s);
	//tm_chan.tm_sec++;
	printf("          |    System| Chantilly|\n"); 
	printf("Day       |        %02d|",tm_now.tm_mday);
	if (tm_chan.tm_mday==tm_now.tm_mday) printf(ANSI_COLOR_GREEN"        %02d"ANSI_COLOR_RESET"|\n",tm_chan.tm_mday);
	else {
		printf(ANSI_COLOR_RED"        %02d"ANSI_COLOR_RESET"|\n",tm_chan.tm_mday);
		return 0;
	}       
	printf("Month     |        %02d|",tm_now.tm_mon+1);
	if (tm_chan.tm_mon+1==tm_now.tm_mon+1) printf(ANSI_COLOR_GREEN"        %02d"ANSI_COLOR_RESET"|\n",tm_chan.tm_mon+1);
	else {
		printf(ANSI_COLOR_RED"        %02d"ANSI_COLOR_RESET"|\n",tm_chan.tm_mon+1);
		return 0;
	}       
	printf("Year      |      %04d|",tm_now.tm_year+1900);
	if (tm_chan.tm_year+1900==tm_now.tm_year+1900) printf(ANSI_COLOR_GREEN"      %04d"ANSI_COLOR_RESET"|\n",tm_chan.tm_year+1900);
	else {
		printf(ANSI_COLOR_RED"      %04d"ANSI_COLOR_RESET"|\n",tm_chan.tm_year+1900);
		return 0;
	}  
	printf("Hour      |        %02d|",tm_now.tm_hour);
	if (tm_chan.tm_hour==tm_now.tm_hour) printf(ANSI_COLOR_GREEN"        %02d"ANSI_COLOR_RESET"|\n",tm_chan.tm_hour);
	else {
		printf(ANSI_COLOR_RED"        %02d"ANSI_COLOR_RESET"|\n",tm_chan.tm_hour);
		return 0;
	}   
	printf("Min       |        %02d|",tm_now.tm_min);
	if (tm_chan.tm_min==tm_now.tm_min) printf(ANSI_COLOR_GREEN"        %02d"ANSI_COLOR_RESET"|\n",tm_chan.tm_min);
	else {
		printf(ANSI_COLOR_RED"        %02d"ANSI_COLOR_RESET"|\n",tm_chan.tm_min);
		return 0;
	}   
	printf("Sec       |        %02d|",tm_now.tm_sec);
	if (tm_chan.tm_sec==tm_now.tm_sec) {
		printf(ANSI_COLOR_GREEN"        %02d"ANSI_COLOR_RESET"|\n",tm_chan.tm_sec);
		return 1;
	}
	else {
		printf(ANSI_COLOR_RED"        %02d"ANSI_COLOR_RESET"|\n",tm_chan.tm_sec);
		return 0;
	}              
}

int8_t testInputs(uint8_t i,uint32_t* cErrors) {
	char c=0;
	uint8_t myRXBuffer[256];
	uint8_t val=0x80;
	uint8_t j=0;
	static uint8_t startValue=0xFF;
	if (i==0) {
		printf(ANSI_COLOR_YELLOW"** INPUTS TEST **\n"ANSI_COLOR_RESET);
		printf("Checking inputs state :  Check all "ANSI_COLOR_YELLOW"dip switches "ANSI_COLOR_RESET"are open and "ANSI_COLOR_YELLOW"push buttons "ANSI_COLOR_RESET"released....");
		do {
			*cErrors+=CHAN_addr(_BUS_BOARD,0x130);
			*cErrors+=CHAN_getBytes(_BUS_BOARD,1,&myRXBuffer[0]);
			//if (myRXBuffer[0]!=0xFD) printf(ANSI_COLOR_YELLOW"."ANSI_COLOR_RESET);
			printf(ANSI_COLOR_YELLOW"."ANSI_COLOR_RESET);
			fflush(stdout);
			sleep(1);
			j++;
		}while (myRXBuffer[0]<0xFE&&j<10);
		startValue=myRXBuffer[0];
		printf("\n");
		if (j==10) {
			printf("Timeout, try again ? y/n\n");
			while (c!='n' && c!='y') {
				getch_start();																//
				c=getch_try();																//
				getch_stop();
			}	
			if (c=='n') {
				printf(ANSI_COLOR_RED"INPUTS TEST INITIALIZATION KO"ANSI_COLOR_RESET);
				return 0;
			}
			else if (c=='y') return testInputs(i,cErrors);
		}
	}
	if (i==0) i++;
	if (i!=5) {
		j=0;
		printf("Close "ANSI_COLOR_YELLOW"dip switch %d "ANSI_COLOR_RESET"or hold "ANSI_COLOR_YELLOW"button %d "ANSI_COLOR_RESET"pressed...",i,i);
		do {
			*cErrors+=CHAN_addr(_BUS_BOARD,0x130);
			*cErrors+=CHAN_getBytes(_BUS_BOARD,1,&myRXBuffer[0]);
			//if (myRXBuffer[0]!=startValue-(val>>(i-1))) printf(ANSI_COLOR_YELLOW"."ANSI_COLOR_RESET);
			printf(ANSI_COLOR_YELLOW"."ANSI_COLOR_RESET);
			fflush(stdout);
			sleep(1);
			j++;
		}while (myRXBuffer[0]!=startValue-(val>>(i-1))&&j<10);
		printf("\n");
		if (j==10) {
			printf("Timeout or wrong value, try again ? y/n\n");
			while (c!='n' && c!='y') {
				getch_start();																//
				c=getch_try();																//
				getch_stop();
			}	
			if (c=='n') {
				if (i<5) printf(ANSI_COLOR_RED"INPUT %c TEST KO\n"ANSI_COLOR_RESET,72-i);
				else printf(ANSI_COLOR_RED"INPUT %c TEST KO\n"ANSI_COLOR_RESET,71-i);	
				return 0;
			}
			else if (c=='y') return testInputs(i,cErrors);
		}
		j=0;
		printf("Open "ANSI_COLOR_YELLOW"dip switch %d "ANSI_COLOR_RESET"or release "ANSI_COLOR_YELLOW"button %d"ANSI_COLOR_RESET"...",i,i);
		do {
			*cErrors+=CHAN_addr(_BUS_BOARD,0x130);
			*cErrors+=CHAN_getBytes(_BUS_BOARD,1,&myRXBuffer[0]);
			//if (myRXBuffer[0]!=startValue) printf(ANSI_COLOR_YELLOW"."ANSI_COLOR_RESET);
			printf(ANSI_COLOR_YELLOW"."ANSI_COLOR_RESET);
			fflush(stdout);
			sleep(1);
			j++;
		}while (myRXBuffer[0]!=startValue&&j<10);
		printf("\n");
		if (j==10) {
			printf("Timeout, try again ? y/n\n");
			while (c!='n' && c!='y') {
				getch_start();																//
				c=getch_try();																//
				getch_stop();
			}	
			if (c=='n') {
				//if (i<5) printf(ANSI_COLOR_RED"INPUT %c TEST KO\n"ANSI_COLOR_RESET,72-i);
				//else printf(ANSI_COLOR_RED"INPUT %c TEST KO\n"ANSI_COLOR_RESET,71-i);
				printf(ANSI_COLOR_RED"INPUT %c TEST KO\n"ANSI_COLOR_RESET,72-i);
				return 0;
			}
			else if (c=='y') return testInputs(i,cErrors);
		}
		if (myRXBuffer[0]==startValue&&i<8) {     // will be 6 
			//if (i<5) printf(ANSI_COLOR_GREEN"INPUT %c TEST OK\n"ANSI_COLOR_RESET,72-i);
			//else printf(ANSI_COLOR_GREEN"INPUT %c TEST OK\n"ANSI_COLOR_RESET,72-i);
			printf(ANSI_COLOR_GREEN"INPUT %c TEST OK\n"ANSI_COLOR_RESET,72-i);
			if (i<7) return testInputs(++i,cErrors);  // will be 6 
			else return 1;
		}
		else return 0;
	}
	else return testInputs(++i,cErrors);
}

int main (int argc,char *argv[ ]) {
	
	int32_t errSetup=0;
	uint32_t chanErrors=0;
	
	//char* t=NULL;
	//char c=0;
	
	errSetup=CHAN_setup("chConnectionTest",1);
	switch (errSetup) {
		case 0://.......................................................CHAN_SETUP() is successfull 
			printf("Immediate mode is set.\n\n");
		break;
		case -1://......................................................Daemon runs
			printf("ioDAEMON is running, please kill it before running this program.\n");
			exit(EXIT_SUCCESS);
		break;
		case -2://......................................................other errors
			printf("Something wrong has happened, program will close now.\n");
			//CHAN_close();
			exit(EXIT_FAILURE);
		break;
		default :														//The process, which PID is returned, is allready uses immediat mode.
			printf("Process with PID=%d is using chantilly in immediat mode, please kill it before running this program.\n",errSetup);
			//CHAN_close();
			exit(EXIT_SUCCESS);
		break;
	}/**/
	if (!testRTC(&chanErrors)) printf(ANSI_COLOR_RED"** RTC TEST KO **\n\n"ANSI_COLOR_RESET);
	else {
		printf(ANSI_COLOR_GREEN"** RTC TEST OK **\n\n"ANSI_COLOR_RESET);
		if (!testSelectCore(0)) printf(ANSI_COLOR_RED"** SELECT CORES TEST KO **\n\n"ANSI_COLOR_RESET);
		else { 
			printf(ANSI_COLOR_GREEN"** SELECT CORES TEST OK **\n\n"ANSI_COLOR_RESET);
			if (!testInputs(0,&chanErrors)) printf(ANSI_COLOR_RED"** INPUTS TEST KO **\n"ANSI_COLOR_RESET);
			else printf(ANSI_COLOR_GREEN"** INPUTS TEST OK **\n"ANSI_COLOR_RESET);
		}
	}
	printf("** END TEST **\n");
	chanErrors+=CHAN_close();
	printf("\naccess errors : %d\n\n",chanErrors);		// Displays access errors (optionnal for debug).
	fflush(stdout);
	return 0;
	
}
