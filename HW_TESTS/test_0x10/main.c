#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
 
#include <CHANTILLY_BP.h>
#include "/home/pi/code_004/common/CHAN/daemon_defs.h"
#include "/home/pi/code_004/common/CHAN/daemon_utils.h"

#define _THIS_APP_ID 			0xFF

#define TESTING_CORE_NUM 		2
#define TESTED_CORE_NUM 		1 						// IO24 is core number 1 on chantilly stack. (Choosen by user but must match hardware-address on IO24 Board)

#define TEST_CORE_ACCESS_V		139
#define TEST_CORE_ID			0x10
#define TEST_CORE_FIRMWARE_V	5
 
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

int main (int argc,char *argv[ ]) {
	//bus_struct chantilly_stack;
	uint8_t i=0;
	int32_t errSetup=0;
	uint32_t chanErrors=0;
	uint8_t bufRX[256]={0};
	uint8_t bufTX[256]={0};
	int8_t c=0;
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
	}	
	chanErrors+=CHAN_addr(TESTED_CORE_NUM,0x1E9);
	chanErrors+=CHAN_getBytes(TESTED_CORE_NUM,3,bufRX);
	printf("IO24 header reading : \n");
	if (bufRX[1]==TEST_CORE_ID) printf("Id : "ANSI_COLOR_GREEN"0x%X\n"ANSI_COLOR_RESET,bufRX[1]);
	else printf("Id : "ANSI_COLOR_RED"0x%X\n"ANSI_COLOR_RESET,bufRX[1]);
	
	if (bufRX[0]==TEST_CORE_ACCESS_V) printf("access version : "ANSI_COLOR_GREEN"%d\n"ANSI_COLOR_RESET,bufRX[0]);
	else printf("access version : "ANSI_COLOR_GREEN"%d\n"ANSI_COLOR_RESET,bufRX[0]);
	
	if (bufRX[2]==TEST_CORE_FIRMWARE_V) printf("firmware version : "ANSI_COLOR_GREEN"%d\n"ANSI_COLOR_RESET,bufRX[2]);
	else printf("firmware version : "ANSI_COLOR_RED"%d\n"ANSI_COLOR_RESET,bufRX[2]);
	
	if (bufRX[1]==TEST_CORE_ID&&bufRX[0]==TEST_CORE_ACCESS_V&&bufRX[2]==TEST_CORE_FIRMWARE_V) {
		printf (ANSI_COLOR_GREEN"IO24 header reading is OK.\n"ANSI_COLOR_RESET);
	}
	else {
		printf (ANSI_COLOR_RED"IO24 header reading is KO.\n"ANSI_COLOR_RESET);
		//CHAN_close();
		//exit(EXIT_FAILURE);
	}
	chanErrors+=CHAN_command(TESTED_CORE_NUM,0x06);
	chanErrors+=CHAN_command(TESTING_CORE_NUM,0x06);
	bufTX[0]=0xFF;													// inverts input polarity.
	bufTX[1]=0xFF;
	chanErrors+=CHAN_addr(TESTED_CORE_NUM,0x106);
	chanErrors+=CHAN_setBytes(TESTED_CORE_NUM,2,bufTX);
	bufTX[0]=0xFF;													// inverts input polarity.
	bufTX[1]=0xFF;
	chanErrors+=CHAN_addr(TESTING_CORE_NUM,0x106);
	chanErrors+=CHAN_setBytes(TESTING_CORE_NUM,2,bufTX);
	chanErrors+=CHAN_command(TESTED_CORE_NUM,0x01);
	chanErrors+=CHAN_command(TESTING_CORE_NUM,0x01);
	printf("\nIO24 outputs test : \n");
	printf("Put diodes on OutputB bit0 and bit1 on tested, then type a key.\n");
	while (!c) {
		getch_start();																//
		c=getch_try();																//
		getch_stop();
	}		
	bufTX[0]=0xFF;
	bufTX[1]=0xFF;
	chanErrors+=CHAN_addr4(TESTED_CORE_NUM,0x40);
	chanErrors+=CHAN_setBytes(TESTED_CORE_NUM,2,bufTX);
	chanErrors+=CHAN_addr(TESTING_CORE_NUM,0x102);
	usleep(100000);
	chanErrors+=CHAN_getBytes(TESTING_CORE_NUM,2,bufRX);
	printf("Tested OutputA = 0x%02X\t",bufTX[0]);
	printf("Testing InputB = 0x%02X\t",bufRX[1]);
	fflush(stdout);
	if (bufRX[1]!=0xFF) {
		printf(ANSI_COLOR_RED"OutputA all bits high KO\n"ANSI_COLOR_RESET);
		//CHAN_close();
		//exit(EXIT_FAILURE);
	}
	else printf(ANSI_COLOR_GREEN"OutputA all bits high OK\n"ANSI_COLOR_RESET);
	printf("Tested OutputB = 0x%02X\t",bufTX[1]);
	printf("Testing InputA = 0x%02X\t",bufRX[0]);
	fflush(stdout);
	if (bufRX[0]!=0xFC) {
		printf(ANSI_COLOR_RED"OutputB all bits high KO\n"ANSI_COLOR_RESET);
		//CHAN_close();
		//exit(EXIT_FAILURE);
	}
	else printf(ANSI_COLOR_GREEN"OutputB all bits high OK\n"ANSI_COLOR_RESET);
	bufTX[0]=0;
	bufTX[1]=0;
	chanErrors+=CHAN_addr4(TESTED_CORE_NUM,0x40);
	chanErrors+=CHAN_setBytes(TESTED_CORE_NUM,2,bufTX);
	chanErrors+=CHAN_addr(TESTING_CORE_NUM,0x102);
	usleep(100000);
	chanErrors+=CHAN_getBytes(TESTING_CORE_NUM,2,bufRX);
	printf("Tested OutputA = 0x%02X\t",bufTX[0]);
	printf("Testing InputB = 0x%02X\t",bufRX[1]);
	fflush(stdout);
	if (bufRX[1]!=0) {
		printf(ANSI_COLOR_RED"OutputA all bits low KO\n"ANSI_COLOR_RESET);
		//CHAN_close();
		//exit(EXIT_FAILURE);
	}
	else printf(ANSI_COLOR_GREEN"OutputA all bits low OK\n"ANSI_COLOR_RESET);
	printf("Tested OutputB = 0x%02X\t",bufTX[1]);
	printf("Testing InputA = 0x%02X\t",bufRX[0]);
	fflush(stdout);
	if (bufRX[0]!=0) {
		printf(ANSI_COLOR_RED"OutputB all bits low KO\n"ANSI_COLOR_RESET);
		//CHAN_close();
		//exit(EXIT_FAILURE);
	}
	else printf(ANSI_COLOR_GREEN"OutputB all bits low OK\n"ANSI_COLOR_RESET);
	bufTX[0]=1;
	bufTX[1]=1;
	for (i=0;i<8;i++) {
		chanErrors+=CHAN_addr4(TESTED_CORE_NUM,0x40);
		chanErrors+=CHAN_setBytes(TESTED_CORE_NUM,2,bufTX);
		chanErrors+=CHAN_addr(TESTING_CORE_NUM,0x102);
		usleep(100000);
		chanErrors+=CHAN_getBytes(TESTING_CORE_NUM,2,bufRX);
		printf("Tested OutputA = 0x%02X\t",bufTX[0]);
		printf("Testing InputB = 0x%02X\t",bufRX[1]);
		fflush(stdout);
		if (bufTX[0]!=bufRX[1]) {
			printf(ANSI_COLOR_RED"OutputA bit%d KO\n"ANSI_COLOR_RESET,i);
			//CHAN_close();
			//exit(EXIT_FAILURE);
		}
		else printf(ANSI_COLOR_GREEN"OutputA bit%d OK\n"ANSI_COLOR_RESET,i);
		if (i<2) {
			printf("Tested OutputB = 0x%02X\t",bufTX[1]);
			printf("Diode %d is on ? y/n\t",i);
			fflush(stdout);
			while (c!='n' && c!='y') {
				getch_start();																//
				c=getch_try();																//
				getch_stop();
			}	
			if (c=='n') {
				printf(ANSI_COLOR_RED"OutputB bit%d KO\n"ANSI_COLOR_RESET,i);
				//CHAN_close();
				//exit(EXIT_FAILURE);
			}
			else if (c=='y') printf(ANSI_COLOR_GREEN"OutputB bit%d OK\n"ANSI_COLOR_RESET,i);
			c=' ';
		}
		else {
			printf("Tested OutputB = 0x%02X\t",bufTX[1]);
			printf("Testing InputA = 0x%02X\t",bufRX[0]);
			fflush(stdout);
			if (bufRX[0]!=bufTX[1]) {
				printf(ANSI_COLOR_RED"OutputB bit%d KO\n"ANSI_COLOR_RESET,i);
				//CHAN_close();
				//exit(EXIT_FAILURE);
			}
			else printf(ANSI_COLOR_GREEN"OutputB bit%d OK\n"ANSI_COLOR_RESET,i);
		}
		bufTX[0]=bufTX[0]<<1;
		bufTX[1]=bufTX[1]<<1;
	} 
	printf("\nIO24 intputs test : \n");
	///all bits high
	bufTX[0]=0xFF;
	bufTX[1]=0xFF;
	chanErrors+=CHAN_addr4(TESTING_CORE_NUM,0x40);
	chanErrors+=CHAN_setBytes(TESTING_CORE_NUM,2,bufTX);
	chanErrors+=CHAN_addr(TESTED_CORE_NUM,0x102);
	usleep(1000000);
	chanErrors+=CHAN_getBytes(TESTED_CORE_NUM,2,bufRX);
	printf("Testing OutputB = 0x%02X\t",bufTX[1]);
	printf("Tested InputA = 0x%02X\t",bufRX[0]);
	fflush(stdout);
	if (bufRX[0]!=0xFF) {
		printf(ANSI_COLOR_RED"InputA all bits high KO\n"ANSI_COLOR_RESET);
		//CHAN_close();
		//exit(EXIT_FAILURE);
	}
	else printf(ANSI_COLOR_GREEN"InputA all bits high OK\n"ANSI_COLOR_RESET);
	printf("Testing OutputA = 0x%02X\t",bufTX[0]);
	printf("Tested InputB = 0x%02X\t",bufRX[1]);
	fflush(stdout);
	if (bufRX[1]!=0xFF) {
		printf(ANSI_COLOR_RED"InputB all bits high KO\n"ANSI_COLOR_RESET);
		//CHAN_close();
		//exit(EXIT_FAILURE);
	}
	else printf(ANSI_COLOR_GREEN"InputB all bits high OK\n"ANSI_COLOR_RESET);
	///all bits low
	bufTX[0]=0;
	bufTX[1]=0;
	chanErrors+=CHAN_addr4(TESTING_CORE_NUM,0x40);
	chanErrors+=CHAN_setBytes(TESTING_CORE_NUM,2,bufTX);
	chanErrors+=CHAN_addr(TESTED_CORE_NUM,0x102);
	usleep(100000);
	chanErrors+=CHAN_getBytes(TESTED_CORE_NUM,2,bufRX);
	printf("Testing OutputB = 0x%02X\t",bufTX[1]);
	printf("Tested InputA = 0x%02X\t",bufRX[0]);
	fflush(stdout);
	if (bufRX[0]!=0) {
		printf(ANSI_COLOR_RED"InputA all bits low KO\n"ANSI_COLOR_RESET);
		//CHAN_close();
		//exit(EXIT_FAILURE);
	}
	else printf(ANSI_COLOR_GREEN"InputA all bits low OK\n"ANSI_COLOR_RESET);
	printf("Testing OutputA = 0x%02X\t",bufTX[0]);
	printf("Tested InputB = 0x%02X\t",bufRX[1]);
	fflush(stdout);
	if (bufRX[1]!=0) {
		printf(ANSI_COLOR_RED"InputB all bits low KO\n"ANSI_COLOR_RESET);
		//CHAN_close();
		//exit(EXIT_FAILURE);
	}
	else printf(ANSI_COLOR_GREEN"InputB all bits low OK\n"ANSI_COLOR_RESET);
	///each bit
	bufTX[0]=1;
	bufTX[1]=1;
	for (i=0;i<8;i++) {
		chanErrors+=CHAN_addr4(TESTING_CORE_NUM,0x40);
		chanErrors+=CHAN_setBytes(TESTING_CORE_NUM,2,bufTX);
		chanErrors+=CHAN_addr(TESTED_CORE_NUM,0x102);
		usleep(100000);
		chanErrors+=CHAN_getBytes(TESTED_CORE_NUM,2,bufRX);
		if (i<2) {
			printf("Testing OutputA = 0x%02X\t",bufTX[0]);
			printf("Tested InputA = 0x%02X\t",bufRX[0]);
			if (bufRX[0]!=bufTX[0]) {
				printf(ANSI_COLOR_RED"InputA bit%d KO\n"ANSI_COLOR_RESET,i);
				//CHAN_close();
				//exit(EXIT_FAILURE);
			}
			else printf(ANSI_COLOR_GREEN"InputA bit%d OK\n"ANSI_COLOR_RESET,i);
		}
		else {
			printf("Testing OutputB = 0x%02X\t",bufTX[1]);
			printf("Tested InputA = 0x%02X\t",bufRX[0]);
			if (bufRX[0]!=bufTX[1]) {
				printf(ANSI_COLOR_RED"InputA bit%d KO\n"ANSI_COLOR_RESET,i);
				//CHAN_close();
				//exit(EXIT_FAILURE);
			}
			else printf(ANSI_COLOR_GREEN"InputA bit%d OK\n"ANSI_COLOR_RESET,i);
		}
		printf("Testing OutputA = 0x%02X\t",bufTX[0]);
		printf("Tested InputB = 0x%02X\t",bufRX[1]);
		if (bufRX[1]!=bufTX[0]) {
			printf(ANSI_COLOR_RED"InputB bit%d KO\n"ANSI_COLOR_RESET,i);
			//CHAN_close();
			//exit(EXIT_FAILURE);
		}
		else printf(ANSI_COLOR_GREEN"InputB bit%d OK\n"ANSI_COLOR_RESET,i);
		bufTX[0]=bufTX[0]<<1;
		bufTX[1]=bufTX[1]<<1;
	}
	printf("\nIO24 access test : \n");
	for (i=0;i<255;i++) {
		chanErrors+=CHAN_addr(TESTED_CORE_NUM,i);
		chanErrors+=CHAN_getBytes(TESTED_CORE_NUM,1,bufRX);
	}
	if (!chanErrors) {
		printf ("access errors : "ANSI_COLOR_GREEN"%d\n"ANSI_COLOR_RESET,chanErrors);
		printf (ANSI_COLOR_GREEN"IO24 access test is OK.\n\n"ANSI_COLOR_RESET);
	}
	else {
		printf ("access errors : "ANSI_COLOR_RED"%d\n"ANSI_COLOR_RESET,chanErrors);
		printf (ANSI_COLOR_RED"IO24 access test is KO.\n\n"ANSI_COLOR_RESET);
		//CHAN_close();
		//exit(EXIT_FAILURE);
	}
	fflush(stdout);
	CHAN_close();
	return 0;
}
