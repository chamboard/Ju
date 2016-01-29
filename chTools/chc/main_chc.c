#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>			// RTC functions + ..
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sched.h>
#include <sys/wait.h>
//#include <fcntl.h>
//#include <math.h>
//#include <sys/mman.h>
//#include <sys/time.h>
//#include <sys/resource.h>
//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
//
#include <CHANTILLY_BP.h>
#include "/home/pi/code_004/common/CHAN/daemon_defs.h"
#include "/home/pi/code_004/common/CHAN/daemon_utils.h"
#include "/home/pi/code_004/UTILS/debug_time.h"      //for debug_time

/*#include "local_config.h"
//
#include "../common/CHAN/debug_utils.h"
#include "../common/CHAN/core_utils.h"
#include "../../common/CHAN/daemon_utils.h"
#include "../common/CHAN/core_list.h"
//
*/
//
//#include "../common/CHAN/coredef_0xBB.h"
//#include "../common/CHAN/coredef_0x10.h"
//#include "../common/CHAN/daemonfunc_0xBB.h"
//#include "../common/CHAN/corefunc_0xBB.h"
//#include "../common/CHAN/corefunc_0x10.h"
//
struct sched_param param;
uint8_t CORE_ID[8]={0};
uint8_t CORE_NAME[8][20];//={('\0')};//{"Bus_Board      \0","IO_24          \0","PWM_PID        \0","\0","\0","\0","\0","\0"};
uint8_t myCORES[8][4];
uint8_t CORE_DEFS[8][24];
uint8_t myRXbuffer[256];
float 	myRXbuffer_f[256];
uint8_t myTXbuffer[256];
int32_t debugLevel=0;
uint8_t myImmediatCORES[8][4];
uint32_t chanErrors=0;
//
#include "func_chc.h"
//
static const char short_options[]="hli:n:a:r::w::e:p:v:fmsqd:xb";
//
static const struct option long_options[]={
		{"help",no_argument,NULL,'h'},
        {"list",no_argument,NULL,'l'},
        {"id",required_argument,NULL,'i'},
        {"name",required_argument,NULL,'n'},
        {"address",required_argument,NULL,'a'},
        {"read",optional_argument,NULL,'r'},
        {"write",optional_argument,NULL,'w'},
        {"execute",required_argument,NULL,'e'},
        {"parameter",required_argument,NULL,'o'},
        {"value",required_argument,NULL,'v'},
        {"functions",no_argument,NULL,'f'},
        {"map",no_argument,NULL,'m'},
        {"start",no_argument,NULL,'s'},
        {"quit",no_argument,NULL,'q'},
        {"debug",required_argument,NULL,'d'},
        {"xxx",no_argument,NULL,'x'},
        {"base",no_argument,NULL,'b'},
        {0,0,0,0}
};
//
static void errno_exit(const char *s)
{
        fprintf(stderr,"%s error %d, %s\n",s,errno,strerror(errno));
        exit(EXIT_FAILURE);
}
//
static void usage(FILE *fp,int argc,char **argv)
{
        fprintf(fp,
                 "Usage: %s [options]\n\n"
                 "Version %d\n"
                 "Options:\n"
                 "-h | --help		Print this message.\n"
                 "-l | --list		Cores list (Identifier, name, type, address).\n"
                 "-i | --id		Core Identifier (0xBB, 0x10,...).\n" 
                 "-n | --name		Core Name (Bus_Board, IO_24, PWM_PID,...).\n"
                 "-a | --address		Core address (from 1 included to 7 included)\n"
                 "-r | --read		Read in core Ram the number of bytes specified by this option at specifed ram address.\n"     // in RAM at address specified with --parameter/-p option.\n"
                 "-w | --write		Write 1 byte in core Ram at specified ram address.\n"                                          // at header definition address.\n"
                 "-e | --execute		Execute the core function specified by this option.\n" 
                 "-p | --parameter	RAM address (from 0x100 included to 0x1E7 included)\n"
                 "-v | --value		RAM address value.\n"
                 "-f | --functions	List low-level core functions (should be used with \"| less\").\n"
                 "-m | --map		List all core Ram bytes (should be used with \"| less\").\n"
                 "-s | --start		Start ioDaemon\n"
                 "-d | --debug		Debug(b0 : ram address ; b1 : cores list ; b2 coredef).\n"
                 "-x | --xxx		dump ram.\n" 
                 "-b | --base		display read values in decimal format\n"
                 "",
                 argv[0],_THIS_VERS);
}
//

int main (int argc,char *argv[ ]) {
	//
	bus_struct chantilly_stack;
	//
	time_t rawtime;
	struct tm * timeinfo;
	char * system_time=NULL;
	//
	struct timespec fps_start;
	struct timespec fps_end;
	uint64_t exec_time=0;
	if (debugLevel&0x100) get_debug_time (0,fps_start,fps_end);
	//
    int8_t idx;
    int8_t c;
    extern char *optarg;
    extern int optind,optopt;
    //
    uint8_t myslot;
	uint8_t myid;
	//
	int16_t i=0;
	uint8_t j=0,k=0,m=0;
	//
	uint8_t options=0;
    uint8_t required_params[16]={0b00001000,0b00000100,0b00000110,0b00000111,0b00000100}; //[read,write,execute]=0b(coreId or coreName or coreAddress and ramAddress and value)
    //
    int32_t errSetup=0;
    uint8_t errId=0,errName=0,errAddress=0,errRamAddress=0,errValue[255]={0},errRead=0,errWrite=0,errExecFunc=0,errMapRam=0,errMapFunc=0,errDumpRam=0; 
    int32_t coreId=0,coreAddress=0,count=1,ramAddress_int=0,value_int[255]={0},execFunc=0,nbBlocsValues=1;
    uint8_t *coreName=NULL,*ramAddress_ptr=NULL,*coreId_ptr=NULL,*values=NULL;
    uint8_t listCores=0,readRam=0,writeRam=0,mapFunc=0,mapRam=0,dumpRam=0,startDaemon=0,quitDaemon=0;
    uint8_t nbValues=0;
    float value[255]={0},value_buf[255]={0};
    uint8_t value_l[255]={0},value_h[255]={0},litteral=0;
	uint8_t * values_ptr=NULL;
	uint8_t decimal=0;
	//
	pid_t pid;
	int pd[2]; //pipe descriptor
	unsigned char messageR[256],messageW[256];
    char *arg[]={"sudo","ioDAEMON",NULL};
    //pcui ppp;
    pcui *ppp;
	//
	clock_gettime(CLOCK_MONOTONIC,&fps_start);
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	system_time= asctime (timeinfo);
	printf("%s\n",system_time);	
	if (setpriority(PRIO_PROCESS, 0, PRIO_MAX)==0) printf("Prioritymax is set\n\n");
	ppp=malloc(sizeof(pcui)*1);
	//
    for (;;) {
		c=(int)getopt_long(argc,argv,short_options,long_options,(int*)&idx);
		if (-1==c) break;
		switch (c) {
			case 0: /* */
			break;
			case 'h':
				usage(stdout,argc,argv);
				exit(EXIT_SUCCESS);
			break;
			case 'l':
				listCores=1;
			break;
			case 'i' :
				errno=0;
				coreId_ptr=(uint8_t*)optarg;
				coreId=strtol(optarg,NULL,0);
				if (errno) errno_exit(optarg);
				if (!coreName) options+=8;
			break;
			case 'n' :
				errno=0;
				coreName=strtoupper((uint8_t*)optarg);
				if (errno) errno_exit(optarg);
				if (!coreId) options+=8;
			break;
			case 'a' :
				errno=0;
				coreAddress=strtol(optarg,NULL,0);
				if (errno) errno_exit(optarg);
				options+=4;
			break;
			case 'r' :
				if (optarg) { //??
					errno=0;
					if (*optarg=='=') count=strtol(optarg+1,NULL,0);
					else count=strtol(optarg,NULL,0);
					//else count=0;
					if (errno) errno_exit(optarg);
				}
				readRam=1;
			break;
			case 'w' :
				if (optarg) { //??
					errno=0;
					if (*optarg=='=') nbBlocsValues=strtol(optarg+1,NULL,0);
					else nbBlocsValues=strtol(optarg,NULL,0);
					//else nbBlocsValues=0;
					if (errno) errno_exit(optarg);
				}
				writeRam=1;
			break;
			case 'e' :
				errno=0;
				execFunc=strtol(optarg,NULL,0) ;
				if (errno) errno_exit(optarg) ;
				options+=16;
			break;
			case 'p' :
				errno=0;
				ramAddress_ptr=strtoupper((uint8_t*)optarg);
				ramAddress_int=strtol(optarg,NULL,0);
                if (errno) errno_exit(optarg);
                options+=2;
			break ;
			case 'v' :
				errno = 0;
				values=(uint8_t*)optarg;
				if (errno) errno_exit(optarg) ;
				options+=1;
			break;
			case 'f' :
				mapFunc=1;
			break;
			case 'm' :
				mapRam=1;
			break;
			case 's' :
				startDaemon=1;
			break;
			case 'q' :
				quitDaemon=1;
			break;
			case 'd' :
				errno=0;
				debugLevel=strtol(optarg,NULL,0);
				if (errno) errno_exit(optarg);
			break;
			case 'x' :
				dumpRam=1;
			break;
			case 'b' :
				decimal=1;
			break;
			case ':':
				printf(":\n");
			case '?':
				printf("?\n");
				printf("optopt : %d\n",optopt);
				printf("optind : %d\n",optind);
			break;
			default:
				//printf("%d",optopt);
				usage(stderr,argc,argv);
				exit(EXIT_FAILURE);
			break;
		}
	}
	parse_core_list();
	if (debugLevel&0x100) get_debug_time (1,fps_start,fps_end);	
	errSetup=CHAN_setup("chc",1); 
	if (debugLevel&0x100) get_debug_time (2,fps_start,fps_end);	
	if (debugLevel&0x02) printf("Setup returned code : %d \t",errSetup);
	switch (errSetup) {
		case 0://.......................................................CHAN_SETUP() is successfull 
			printf("Immediate mode is set.\n");
			if (startDaemon&&quitDaemon) {printf("\nWARNING : using both -s|--start and -q|--quit options.\n");quitDaemon=2;}
		break;
		case -1://......................................................Daemon runs
			if (!startDaemon&&!quitDaemon) {
				do {
					printf("ioDAEMON is running do you want to use it ? y/n : ");
					scanf("%c",&startDaemon);
				} while (startDaemon!='y'&&startDaemon!='Y'&&startDaemon!='n'&&startDaemon!='N');
			}
			else if (!startDaemon&&quitDaemon) startDaemon=1;
			else if (startDaemon&&quitDaemon) {printf("WARNING : using both -s|--s and -q|--quit options.\n");quitDaemon=2;}
			if (startDaemon!='y'&&startDaemon!='Y'&&startDaemon!=1){
				printf("Please kill ioDAEMON to run this program in immediat mode\n");
				//CHAN_close();
				exit(EXIT_SUCCESS);
			}
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
	//
	if (startDaemon) {//................................................if start daemon option : -s|--start . 
		if (errSetup==0) {//............................................if immediat mode is actually running.
			printf("\nClosing immediat mode : ...\n");
			CHAN_close();
			
			/*if(pipe(pd)!=0)
			{
				fprintf(stderr,"Erreur de création du tube.\n");
				return EXIT_FAILURE;
			}*/
			pid=create_process();                            //sudo ioDAEMON
			switch (pid) {
				case -1://..............................................(ENOMEM error)
					perror("fork");
					return EXIT_FAILURE;
				break;
				case 0://...............................................child_process
					//close(pd[0]);
					//sprintf((char*)messageW,"ioDAEMONs");
					//printf("Starting \"%s\" (pid = %d) as child process running in background.\t",messageW,getpid());
					/* Close up standard input of the child */
                close(0);
                
                /* Duplicate the input side of pipe to stdin */
                dup(pd[0]);
                //execlp("ls", "ls", NULL);
					printf("Starting \"ioDAEMON\" (pid = %d) as child process running in background.\n",getpid());
					fflush(stdout);
					//write(pd[1],messageW,256);
					son_process(arg);
					CHAN_commandDAEMON(_DAEMON_SS_SET_SHARED);
				break;
				default://..............................................father_process
					sleep(2);
					//close(pd[1]);
					//read(pd[0],messageR,256);
					printf("Continuing main program (pid = %d) \n",getpid());
				break;
			}
		}
		if (debugLevel&0x100) get_debug_time (3,fps_start,fps_end);
		myid=_THIS_APP_ID;
		myslot=CHAN_getDAEMONslot(myid);//..............................return 0x0A if error no_slot, 0x00..0x0 valid slots, 0x0B if error 
		printf("Slot %d is open.\n\n",myslot);
		if (quitDaemon==1) {
			if (myslot) {
			 	do {
					printf("\nWARNING : current slot has non-zero value, some process could use ioDAEMON are you sure to want to stop it ? y/n : ");
					scanf("%c",&quitDaemon);
				} while (quitDaemon!='y'&&quitDaemon!='Y'&&quitDaemon!='n'&&quitDaemon!='N');
			}
			if (quitDaemon=='y'||quitDaemon=='Y'||quitDaemon==1){
				CHAN_commandDAEMON(_DAEMON_SS_QUIT);
				startDaemon=0;
				printf("Stopping ioDAEMON : ...\t");
				fflush(stdout);
				sleep(2);
				errSetup=CHAN_setup("chc",1); 
				printf("Immediate mode is set.\n");
			}
		}
	}
	if (myslot!=0x0A){
		if (!startDaemon) {
			if (listCores) {
				for (m=1;m<8;m++) {
					CHAN_checkBUS(&chantilly_stack,m,0);
				}
			}
			else {
				if (debugLevel&0x100) get_debug_time (4,fps_start,fps_end);
				if (coreAddress) {
					CHAN_checkBUS(&chantilly_stack,coreAddress,0);
					if (debugLevel&0x100) get_debug_time (5,fps_start,fps_end);	
					if (chantilly_stack.core_id[coreAddress]==0){
						errAddress+=1;	
					}
				}
			}
		}
		else {
			for(i=1;i<8;i++) { 
				//while (ii!=myCORES[ii][0]);
				CHAND_getCOREINFO(i,&myCORES[i][0]);
				chantilly_stack.core_access_v[i]=myCORES[i][1];
				chantilly_stack.core_id[i]=myCORES[i][2];
				chantilly_stack.core_firmware_v[i]=myCORES[i][3];
			}
		}
		
		sprintf((char*)&CORE_DEFS[coreAddress][0],"_0x%02X_%03d_%03d",chantilly_stack.core_id[coreAddress],chantilly_stack.core_access_v[coreAddress],chantilly_stack.core_firmware_v[coreAddress]);
		
		//check if can access a connected core with user command arguments 
		if ((readRam||writeRam||execFunc||mapRam||mapFunc||dumpRam)&&(options<required_params[_ACCESS_ONLINE_CORE])) {errExecFunc+=1;errRead+=1;errWrite+=1;errMapRam+=1;errMapFunc+=1;errDumpRam+=1;}
		else if (options>=required_params[_ACCESS_ONLINE_CORE]){
			if (!coreId&&!coreName) {
				//coreId=myCORES[coreAddress][2];rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
				coreId=chantilly_stack.core_id[coreAddress];
			}
			if ((coreId_ptr||coreId)&&!coreName) {//................................core identifier option : -i|--id   
				i=0;
				while ((coreId!=CORE_ID[i])&&i++<8);
				if (i==9) errId+=1;	
				else coreName=CORE_NAME[i];
			}
			else if (coreName&&!coreId) {
				i=0;
				while (strcmp((const char*)coreName,(const char*)CORE_NAME[i])&&i++<8);
				if (i==9) errName+=1;
				else coreId=CORE_ID[i];
			}
			else if (coreId&&coreName) {
				i=0;
				while ((coreId!=CORE_ID[i])&&i++<8);
				if (i==9) errId+=1;
				sprintf((char*)coreName,"%-15s",coreName);
				i=0;
				while (strcmp((const char*)coreName,(const char*)CORE_NAME[i])&&i++<8);
				if (i==9) errName+=1;
				if (coreId!=CORE_ID[i]) {
					errId+=2;
					errName+=2;
				}
			}
			if (coreAddress<1||7<coreAddress) {
				errAddress+=2;
			}
			//if (coreId!=myCORES[coreAddress][2]) {rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
			if (coreId!=chantilly_stack.core_id[coreAddress]) {
				errAddress+=4;
			}
		}
		if (debugLevel&0x100) get_debug_time (6,fps_start,fps_end);	
		
		if (errId||errName||errAddress||errExecFunc||errRead||errWrite||errMapRam||errMapFunc||errDumpRam) { //if error occurs (too few arguments or invalid name, id or addresse) display it
			printf("\n");	
			if (errId& 0x01) {
				printf("\nId ERROR %d \t:\n",errId&0x01);
				fprintf(stderr,"Option -i|--id = 0x%X is not valid (must be 0xBB, 0x10, 0x20...).\n\n",coreId);
			}
			if (errName&0x01) {
				printf("\nName ERROR %d \t:\n",errName&0x01);	
				fprintf(stderr,"Option -n|--name = %s isn't valid (must be \"Bus_Board\", \"IO_24\", \"PWM_PID\"...).\n\n",coreName);
			}
			if (errName&0x02) {
				printf("\nName ERROR %d \t:\n",errName&0x02);
				fprintf(stderr,"Option -n|--name = %s doesn't match with option -i|--id = 0x%X \n\n",coreName,coreId);
			}
			if (errAddress&0x01) {
				printf("\nAddress ERROR : \nOption -a|--address = %d isn't valid, no connected core here.\n\n",coreAddress);
			}			
			if (errAddress&0x02) {
				printf("\nAddress ERROR %d \t:\n",errAddress&0x01);
				fprintf(stderr,"Option -a|--address = %d is out of range (must be set between 1 and 7).\n\n",coreAddress);
			}
			if ((errAddress&0x04)&&!errName&&!errId) {
				printf("\nAddress ERROR %d \t:\n",errAddress&0x04);
				fprintf(stderr,"Option -a|--address = %d doesn't match with option -i|--id = 0x%X or option -n|--name = %s.\n\n",coreAddress,coreId,coreName);
			}
			if (readRam) printf("Read aborted.\n");/////////////////////////////////................................
			if (writeRam) printf("Write aborted.\n");///////////////////////////////................................
			if (execFunc) printf("Execute function aborted.\n");////////////////////................................
			if (mapRam) printf ("Map ram aborted.\n");
			if (mapFunc) printf("List functions aborted.\n");
			if (dumpRam) printf("Dump ram aborted.\n");
			fflush(stdout);/**/
		}
		else {           // statements if accessing connected core succeed, checks if other arguments are valid. 
			if (debugLevel&0x100) get_debug_time (7,fps_start,fps_end);
			if (readRam||writeRam||execFunc||mapRam||mapFunc)parse_coredef(&CORE_DEFS[coreAddress][0],ppp); ////44444444444444444444444444444444444444	
			if (debugLevel&0x100) get_debug_time (8,fps_start,fps_end);
			if (readRam ||writeRam||execFunc) {
				if (options<required_params[_EXECUTE]) {errExecFunc+=1;errRead+=1;errWrite+=1;}			 								//too few options for execute 
				else {
					if (!ppp->func[execFunc]) errExecFunc+=2;                                             								//functiun doesn't exist
					if (readRam&&options<required_params[_READ]) {errRead+=1;errWrite+=1;}												//too few options for read
					else if (readRam||writeRam) {
						if (ramAddress_ptr&&strchr((const char*)ramAddress_ptr,'_')) {	
							litteral=1;	
							//printf("litteral : %d\n",litteral);																		//the user specifies textual ramAddress
							i=0;
							while (strcmp((const char*)ramAddress_ptr,(const char*)ppp->txt0[i])&&i++<255);
							//printf ("i : %d\n",i);
							//printf("txt0 : %s\n",ppp->txt0[i]);
							//printf("ptr : %s\n",ramAddress_ptr);
							if ((0<i && i<255)||(!strcmp((const char*)ramAddress_ptr,"_BB_EXTVIN_VOLTS_L")&&i==0)) ramAddress_int=i+256;
							else errRamAddress+=1;																						//textual ramAddress out of range
						}
						else {
							i=ramAddress_int-256;
							//printf("i : %d\n",i);
							if (i>=0) ramAddress_ptr=(uint8_t*)ppp->txt0[i];
							else strcpy((char*)ramAddress_ptr,"none\0");
						}																												//the user specifies numeric ramAddress
						if (ramAddress_int<0x00||0x1f1<ramAddress_int) errRamAddress|=0x1;													//numeric rammAddress out of range
						if (!errRamAddress){
							if (debugLevel&0x10) {
								printf("\n\nram address : %d \n",ramAddress_int);
							}
							if (writeRam&&options<required_params[_WRITE]) errWrite +=1;												//too few options for write
							else if (writeRam) {
								values_ptr=values;
								while(*values++) {	
									if (*values==':'||*values=='\0') {//*values='\0';
										value_buf[j]=strtof((const char*)values_ptr,NULL);
										value_int[j]=strtol((char*)values_ptr, NULL, 0);
										value[j]=value_buf[j];
										j++;
										if (*values==':') values++;
										values_ptr=values;
									}
								}		
								nbValues=j;
								if (nbBlocsValues>1) {
									for (k=1;k<nbBlocsValues;k++) {
										for (j=0;j<nbValues;j++) {
											value_buf[j+k*nbValues]=value_buf[j];
											value_int[j+k*nbValues]=value_int[j];
											value[j+k*nbValues]=value[j];
										}
									}
									nbValues=j*k;
									nbBlocsValues=1;
								}
								if (debugLevel&0x20) {
									printf("\n\nnb values : %d \n",nbValues);			
									for (k=0;k<nbValues;k++) {
									printf ("value_buf[%d] : %f\n",k,value_buf[k]) ;
									printf ("value_int[%d] : %d\n",k,value_int[k]) ;
								}	
								}			
								for (j=0;j<nbValues;j++) {
									if (ppp->type[i+j]==1&&strstr((const char*)ramAddress_ptr,"_L\0")&&litteral&&nbValues<2) {  
										printf("\n\nentering ram address label of a 16bits field (\"_L\" suffixed) implies the formatting of value before writting.\n"); 
										if (value[j]<ppp->low_limit[i+j]||ppp->high_limit[i+j]<value[j]) errValue[j]+=1;								//formated value out of range
										else {
											value[j]*=ppp->gain[i+j]/ppp->step[i+j];
											if (strstr((const char*)ppp->format[i+j],"_16BIT_FLOAT")) {
												value_h[j]=((uint16_t)value[j]>>8)&0x03;
											}
											else if (strstr((const char*)ppp->format[i+j],"_16BIT_16BIT")) {
												value_h[j]=((uint16_t)value[j]>>8)&0xFF;
											}
										}
									}
									else if (ppp->type[i+j]==2) {if (value_int[j]<0||ppp->count_params[i+j]-1<value_int[j]) errValue[j]+=2;}
									else if (ppp->type[i+j]==3) {if (value_int[j]<0||1<value_int[j]) errValue[j] +=4;} 
									else {if (value_int[j]<0||255<value_int[j]) errValue[j] +=8;}
									value_l[j]=(uint8_t)value[j]&0xFF;
								}
							}	
						}
					}
				}
			}
		}
		if (debugLevel&0x100) get_debug_time (9,fps_start,fps_end);	
		if (errRamAddress) {
			printf("\n\nRam address ERROR %d :\n",errRamAddress);
			if (errRamAddress&0x01) fprintf(stderr,"Option -p|--parameter = 0x%X isn't valid or not specified (must be set between 0x100 included and 0x1E7 included or must be a valid label).\n",ramAddress_int);
			if (readRam) printf("Can't read.\n");/////////////////////////////////................................
			if (writeRam) printf("Can't write.\n");///////////////////////////////................................
		}
		for (j=0;j<nbValues;j++) {
			if (errValue[j]) {
				if (errValue[j]&0x01) {
					printf("\nValue ERROR %d : \n",errValue[j]&0x01);
					fprintf(stderr,"Option -v|--value = %f at ram address %s=0x%X is out of range (must be set between %f included and %f included).\n",value_buf[j],ppp->txt0[i+j],ppp->addr0[i+j],ppp->low_limit[i+j],ppp->high_limit[i+j]);//ramAddress_ptr,ramAddress_int
				}
				else if (errValue[j]&0x02) {
					printf("\n\nValue ERROR %d : \n",errValue[j]&0x02);
					fprintf(stderr,"Option -v|--value = %f at ram address %s=0x%X is out of range (must be set between 0 included and %d included).\n",value_buf[j],ppp->txt0[i+j],ppp->addr0[i+j],ppp->count_params[i+j]-1);
				}
				else if (errValue[j]&0x04) {
					printf("\n\nValue ERROR %d : \n",errValue[j]&0x04);
					fprintf(stderr,"Option -v|--value = %f at ram address %s=0x%X is out of range (must be set to 0 or 1).\n",value_buf[j],ppp->txt0[i+j],ppp->addr0[i+j]);
				}
				else if (errValue[j]&0x08) {
					printf("\n\nValue ERROR %d : \n",errValue[j]&0x08);
					fprintf(stderr,"Option -v|--value = %f at ram address %s=0x%X is out of range (must be set between 0 included and 255 included).\n",value_buf[j],ppp->txt0[i+j],ppp->addr0[i+j]);
				}
				if (writeRam) printf("\n\nCan't write.\n");//////////////////////..................................
			}
		}
		if (errRead&&readRam) {
			printf("\n\nRead ERROR %d :\n",errRead);
			if (errRead&0x01) fprintf(stderr,"Too few arguments for Option -r|--read.\n");
		}
		if (errWrite&&writeRam) {
			printf("\nWrite ERROR %d :\n",errWrite);
			if (errWrite&0x01) fprintf(stderr,"Too few arguments for Option -w|--write.\n");
		}
		if (errExecFunc&&execFunc) {
			printf("\nExecute ERROR %d :\n",errExecFunc);
			if (errExecFunc&0x01) fprintf(stderr,"Too few arguments for Option -e|--execute.\n");
			if (errExecFunc&0x02) fprintf(stderr,"Option -e|--execute = 0x%X is not define for %.10s[id=0x%X/address=%d].\n",execFunc,coreName,coreId,coreAddress);//invalid function
		}
		if (errMapRam&&mapRam) {
			printf("\nMap ram ERROR %d :\n",errMapRam);
			if (errMapRam&0x01) fprintf(stderr,"Too few arguments for Option -m|--map.\n");
		}
		if (errMapFunc&&mapFunc) {
			printf("\nList functions ERROR %d :\n",errMapFunc);
			if (errMapFunc&0x01) fprintf(stderr,"Too few arguments for Option -f|--functions.\n");
		}
		if (errDumpRam&&dumpRam) {
			printf("\nDump ram ERROR %d :\n",errDumpRam);
			if (errDumpRam&0x01) fprintf(stderr,"Too few arguments for Option -x|--xxx.\n");
		}
		if (debugLevel&0x100) get_debug_time (10,fps_start,fps_end);	
		if (writeRam&&!errRamAddress&&!errWrite&&!errId&&!errName&&!errAddress) {    // !errValue_b&&
			if (nbValues==1&&nbBlocsValues==1) {
				if (!errValue[0]) {
					if (strstr((const char*)ramAddress_ptr,"_L\0")) {                                                            //the user specifies textual ramAddress "_L" terminated => formatting value.
						printf("\n\nWrites value=%f at ram address %s=0x%X in %.10s[id=0x%X/address=%d].\n",value[0],ramAddress_ptr,ramAddress_int,coreName,coreId,coreAddress);
						if (startDaemon) {
							formatWrite_CORE((uint8_t) myslot,(uint8_t)coreAddress,(uint16_t)ramAddress_int,(uint8_t)value_l[0],(uint8_t)value_h[0]);
							if (debugLevel&0x40) printf("shm mode.\n");
						}
						else {
							formatWrite_CORE_I((uint8_t)coreAddress,(uint16_t)ramAddress_int,(uint8_t)value_l[0],(uint8_t)value_h[0]);
							if (debugLevel&0x40) printf("Immediat mode.\n");
						}
						
					}
					else {																										//in all other cases raw value is written.
						printf("\n\nWrites value=0x%02X at ram address %s=0x%X in %.10s[id=0x%X/address=%d].\n",(unsigned int)value[0],ramAddress_ptr,ramAddress_int,coreName,coreId,coreAddress);
						if (startDaemon) {
							write_CORE((uint8_t)myslot,(uint8_t)coreAddress,(uint16_t)ramAddress_int,(uint8_t)value_l[0]);
							if (debugLevel&0x40) printf("shm mode.\n");
						}
						else {
							write_CORE_I((uint8_t)coreAddress,(uint16_t)ramAddress_int,value_l[0]);
							if (debugLevel&0x40) printf("Immediat mode.\n");
						}
					}
				}
			}
			else {
				for (j=0;j<nbValues;j++) {
					if (!errValue[j]) {
						printf("\n\nWrites value=0x%02X at ram address %s=0x%X in %.10s[id=0x%X/address=%d].\n",(unsigned int)value[j],ppp->txt0[i+j],ramAddress_int+j,coreName,coreId,coreAddress);
						if (startDaemon) {
							write_CORE((uint8_t)myslot,(uint8_t)coreAddress,(uint16_t)(ramAddress_int+j),(uint8_t)value_l[j]);
							if (debugLevel&0x40) printf("shm mode.\n");
						}
						else {
							write_CORE_I((uint8_t)coreAddress,(uint16_t)(ramAddress_int+j),(uint8_t)value_l[j]);
							if (debugLevel&0x40) printf("Immediate mode.\n");
						}
					}
				}
			}
		}
		if (execFunc&&!errExecFunc&&!errId&&!errName&&!errAddress) {
			printf("\n\nExecutes function : %s=0x%X in %.10s[id=0x%X/address=%d].\n",ppp->txtFunc[execFunc],execFunc,coreName,coreId,coreAddress);
			if (startDaemon) {
				CHAND_doCOMMAND(myslot,coreAddress,execFunc,_DEXEC_COMPLETE);
				if (debugLevel&0x40) printf("shm mode.\n");
			}
			else {
				CHAN_command(coreAddress,execFunc);	
				if (debugLevel&0x40) printf("Immediat mode.\n");
			}
		}
		if (readRam&&!errRamAddress&&!errRead&&!errId&&!errName&&!errAddress) {
			if (strcmp((const char*)ramAddress_ptr,"none\0")) printf("\n\nReads %d byte(s) at ram address %s=0x%X in %.10s[id=0x%X/address=%d]:\n",count,ramAddress_ptr,ramAddress_int,coreName,coreId,coreAddress);
			else printf("\n\nReads %d byte(s) at ram address 0x%X in %.10s[id=0x%X/address=%d]:\n",count,ramAddress_int,coreName,coreId,coreAddress);
			if (startDaemon) {
				if (debugLevel&0x40) printf("shm mode.\n");
				read_CORE((uint8_t)myslot,(uint8_t)coreAddress,(uint16_t)ramAddress_int,(uint8_t)count,ppp);
			}
			else {
				if (debugLevel&0x40) printf("Immediat mode.\n");
				read_CORE_I((uint8_t)coreAddress,(uint16_t)ramAddress_int,(uint8_t)count,decimal,ppp);
			}
		}
		if (listCores) {
			printf("\n\nLists connected cores :\n");
			if (startDaemon) {if (debugLevel&0x40) printf("shm mode.\n");}
			else {if (debugLevel&0x40) printf("Immediat mode.\n");}
			list_connected_CORES(&chantilly_stack);
		}
		if (mapRam&&!errMapRam) {
			printf("\n\nMaps Ram of %.10s[id=0x%X/address=%d] :\n",coreName,coreId,coreAddress);
			if (startDaemon) {
				if (debugLevel&0x40) printf("shm mode.\n");
				mapRam_CORE((uint8_t)myslot,(uint8_t)coreAddress,ppp);
			}
			else {
				if (debugLevel&0x40) printf("Immediat mode.\n");
				mapRam_CORE_I((uint8_t)coreAddress,ppp);
			}
		}
		if (mapFunc&&!errMapFunc) {
			printf("\n\nLists functions of %.10s[id=0x%X/address=%d] :\n",coreName,coreId,coreAddress);
			if (startDaemon) {if (debugLevel&0x40) printf("shm mode.\n");} 
			else {if (debugLevel&0x40) printf("Immediat mode.\n");}
			mapFunc_CORE(ppp);
		}
		if (dumpRam&&!errDumpRam) {
			printf("\n\nDumps Ram of %.10s[id=0x%X/address=%d] :\n",coreName,coreId,coreAddress);
			if (startDaemon) {
				if (debugLevel&0x40) printf("shm mode.\n");
				dumpRam_CORE((uint8_t)myslot,(uint8_t)coreAddress);
			}
			else {
				if (debugLevel&0x40) printf("Immediat mode.\n");
				dumpRam_CORE_I((uint8_t)coreAddress);
			}
		}
		if (quitDaemon==2) {
			if (myslot) {
			 	do {
					printf("\n\nWARNING : current slot has non-zero value, some process could use ioDAEMON are you sure to want to stop it ? y/n : ");
					scanf("%c",&quitDaemon);
				} while (quitDaemon!='y'&&quitDaemon!='Y'&&quitDaemon!='n'&&quitDaemon!='N');
			}
			if (quitDaemon=='y'||quitDaemon=='Y'||quitDaemon==2){
				CHAN_commandDAEMON(_DAEMON_SS_QUIT);
				printf("\n\nStopping ioDAEMON : ...\t");
				fflush(stdout);
				usleep(200000);
				printf("\n\n");
			}
			else {
				printf("\n\n");CHAN_releaseDAEMONslot(myslot,myid);printf("\n");
			}
		}
		else if (startDaemon) {printf("\n\n");CHAN_releaseDAEMONslot(myslot,myid);printf("\n");}
		else {
			printf("\n\n");
			CHAN_close();
			printf("\n");
		}
	}
	else {//if (myslot==0x0A) 
		fprintf(stderr,"ERROR : No avalaible slot or daemon failure, you should try to release some slots closing apps using its.\n");
        exit(EXIT_FAILURE);
	}
	printf("errors : %d\n",chanErrors);
	if (debugLevel&0x100) get_debug_time (11,fps_start,fps_end);
	clock_gettime(CLOCK_MONOTONIC,&fps_end);
	exec_time=(uint64_t)fps_end.tv_nsec+(uint64_t)fps_end.tv_sec*1000000000-(uint64_t)fps_start.tv_sec*1000000000-(uint64_t) fps_start.tv_nsec;
	printf("execution time : %010lu ns\n\n",(unsigned long)exec_time);
	return 0;
}
			
			
			
		
		
