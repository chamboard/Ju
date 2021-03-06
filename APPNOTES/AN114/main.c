 /*******************************************************************
 *                                                                  * 
 * Chantilly 0x10 - IO24 : Source code for application note AN114   *
 *                                                     			    *
 ****************************************************************************************************************************************************************************************************************************************************
 * CIRCUIT DESCRIPTION :																																					                                                                        *
 * Core module IO24 is set at number 1 on chantilly stack (redefine IO24_NUM according to hardware-address to change it : between 1 included and 6 included).																						*
 * As described in file chantilly_0x10_AN114_pcb2rev1.PDF, three 4 wires stepper motors are wired on OutputA and MSBs of OutputB.																													*
 * 																																																													*
 *  																																																												* 
 * PROGRAM DESCRIPTION : Test using IO24 direct core access mode with (immediate inputs and) stepcount sequenced outputs in mode 0.                                                                    		                                       	*
 * A couple of synchronous sequences is written in free RAM.																																														* 
 * Due to mode 0, sequencers Seq play the first sequence of the couple on OutputA and the second sequence of the couple on OutputB.  																												*
 * SEQ_MASK, SEQ1_MASk values make Seq outputted on all bits of OutputA and on all bits of OutputB.																																					*
 *																																																													*
 *  												                                                   																																				*		
 *                                       									                  													                                                   													*
 * Commented lines mainly refers to chantilly use.												                              					                                                   													*
 ***************************************************************************************************************************************************************************************************************************************************/

#define IO24_NUM 1														// IO24 is core number 1 on chantilly stack. (Choosen by user but must match hardware-address on IO24 Board)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include <CHANTILLY_BP.h>
 
typedef struct															// Structure storing common settings to the three sequencers (SEQS_STEPCOUNT_RESTART and SEQ_WRAP and their addresses).
{																		// Even if in mode 0 only sequencer Seq is enforced, SEQS_STEPCOUNT_RESTART and SEQ_WRAP are treated as common setting to the three sequencers.  
	uint8_t stepcountRestart;
	uint32_t stepcountRestartAddr;
	uint8_t wrap;
	uint32_t wrapAddr;
}
sequencers;                                                             

typedef struct															// Structure storing specific settings to each sequencer.
{	
	char name[5];
	uint8_t mask;
	uint32_t maskAddr;
	uint8_t cursor;	
	uint32_t cursorAddr;
	uint8_t astepIncTime;
	uint32_t astepIncTimeAddr;
	uint8_t presetCount;
	uint32_t presetCountAddr;
	uint8_t presetStep;
	uint32_t presetStepAddr;	
}
sequencer;																

	
 /***************************************************************************************************************************************************************************************************************************************************
 *  FUNCTIONS 																																																										*
 ***************************************************************************************************************************************************************************************************************************************************/

 
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

void displayMainMenu() {
	printf("\nMain menu\n");
	printf("Type :\t'0' to display Sequencer Seq menu.\n");
	printf("\t'?' to display status : senses and numbers of remainings steps.\n");
	printf("\t'a' to add 10 steps.\n");
	printf("\t'w' to invert wrap.\n");
	printf("\t'p' to pause/resume.\n");
	printf("\t'q' to exit.\n");
}

uint8_t displaySeqMenu(char c,sequencer* seqN,uint32_t* errors) {							// showMenu() lets the user edit specific settings of the sequencer passed in sequencer.
	uint8_t c2='\0';																		// 
	uint8_t exit_loop=0;																	// 
	uint8_t count=0;
	uint8_t step=0;
	printf("\n\tSequencer %s menu\n",seqN->name);
	printf("\tType :\t'?' to display status : sense and number of remaining steps.");
	printf("\n\t\t'a' to add 10 steps.\n\t\t's' to change sense.\n");
	printf("\t\t'd' to change steps delay.\n\t\t'r' to return.\n\t\t'q' to exit.\n");
	fflush(stdout);
	while (c2!='q'&&c2!='m'&&c2!='d'&&c2!='r'&&c2!='s'&&c2!='a'&&c2!='?') {
		getch_start();																		//
		c2=getch_try();																		//
		getch_stop();																		// Gets the last character typed without stopping program (see FUNCTIONS above).
	}
	if (c2) {
		//printf("c2 : %c\n",c2);
		if (c2=='q') {
			printf("\n");
			exit_loop=1;																	// If character typed is 'q' : Exits loop.
		}
		else {
			if (c2=='a') {																	// If 'a' is typed : adds 10 step to the sequencer passed to the function.  
				*errors+=CHAN_addr(IO24_NUM,seqN->presetCountAddr);							//  Switch seqN parameter passed to the function, sets current address at address of SEQ_PRESET_COUNT or SEQ2_PRESET_COUNT.
				CHAN_getBytes(IO24_NUM,1,&count);											//  Reads value of current address and store it in the variable count.
				*errors+=CHAN_addr(IO24_NUM,seqN->presetCountAddr);							//  Sets current address at address of SEQ_PRESET_COUNT or SEQ2_PRESET_COUNT.
				*errors+=CHAN_setByte(IO24_NUM,seqN->presetCount+count);					//  Writes count+10 at current address.
				if (!count) {																// If count=0. 
					printf("Restarting sequencer %s for 10 steps.\n",seqN->name);			//  Sequencer is stopped and must be restarted.
					errors+=CHAN_command(IO24_NUM,7);										//
				}
				else printf("Remaining steps for sequencer %s : %d\n",						// Else display number of remaining steps of the sequencer passed to the function.
									seqN->name,seqN->presetCount+count); 
			}
			else if (c2=='?') {																// If '?' is typed : displays status of the sequencer passed to the function.
				*errors+=CHAN_addr(IO24_NUM,seqN->presetCountAddr);							//  Sets current address at address of SEQ_PRESET_COUNT or SEQ2_PRESET_COUNT.
				CHAN_getBytes(IO24_NUM,1,&count);											//  Reads value of current address and store it in the variable count.
				printf("Remaining step(s) for sequencer %s : %d\n",seqN->name,count);		//  Display number of remainings steps of the sequencer passed to the function.
				*errors+=CHAN_addr(IO24_NUM,seqN->presetStepAddr);							//  Switch seqN parameter passed to the function, sets current address at address of SEQ_PRESET_STEP or SEQ2_PRESET_STEP.
				CHAN_getBytes(IO24_NUM,1,&step);											//  Reads value of current address and store it in the variable step.
				if (step==0) printf("sequencer %s does nothing\n",seqN->name);				//
				else if (step==1) printf("sequencer %s increases\n",seqN->name);			//
				else if (step==2) printf("sequencer %s decreases\n",seqN->name);			//  Switch value of step displays sense of the sequencer passed to the function. 
			}
			else if (c2=='s') {																// If character typed is 's' : Edits the sense of the sequencer passed to the fuction.
				if (seqN->presetStep==1) seqN->presetStep=2;
				else if (seqN->presetStep==2) seqN->presetStep=0;
				else seqN->presetStep=1;
				*errors+=CHAN_addr(IO24_NUM,seqN->presetStepAddr);							//  Switch seqN parameter passed to the function, sets current address at address of SEQ_PRESET_STEP or SEQ2_PRESET_STEP into IO24.
				*errors+=CHAN_setByte(IO24_NUM,seqN->presetStep);							//  Writes seqN->presetStep at current address.
				*errors+=CHAN_command(IO24_NUM,7);											//  Executes IO24 command 0x07 named STEPCOUNT_SEQUENCER_RUN because SEQ_PRESET_STEP or SEQ2_PRESET_STEP have been edited.
				if (seqN->presetStep==0) printf("sequencer %s does nothing\n",seqN->name);	//
				else if (seqN->presetStep==1) printf("sequencer %s increases\n",seqN->name);//
				else if (seqN->presetStep==2) printf("sequencer %s decreases\n",seqN->name);//  Switch value of step displays the new sense of the sequencer passed to the function.
			}
			else if (c2=='d') {																// If character typed is 'd' : Edits delay between sequence steps . 
				if (c!='0') {																//  If the sequencer passed to the function is not Seq.
					printf("Seq%c steps delay Ts%c is such as",c,c);						//   Displays SEQ2_ASTEP_INC_TIME and current steps delay of the sequencer Seq2.
					printf(" Ts%c=Tseq*(SEQ%c_ASTEP_INC_TIME+1)",c,c);						//
					printf(" with Tseq=5ms and ");											//
					printf("SEQ%c_ASTEP_INC_TIME=%d\n",c,seqN->astepIncTime);				//
					printf("Enter new value for SEQ%c_ASTEP_INC_TIME=",c);					//   Asks a new vale for SEQ2_ASTEP_INC_TIME.
				}                                   
				else {								                                        //  Else 										
					printf("Seq steps delay Ts is such as Ts=Tseq*(SEQ_ASTEP_INC_TIME+1)");	//   Displays SEQ_ASTEP_INC_TIME and current steps delay of the sequencer Seq. 
					printf(" with Tseq=5ms and SEQ_ASTEP_INC_TIME=%d\n",seqN->astepIncTime);//   
					printf("Enter new value for SEQ_ASTEP_INC_TIME=");						//   
				}																			//   Asks a new vale for SEQ_ASTEP_INC_TIME.
				scanf("%3d",(unsigned int*)&seqN->astepIncTime);							// Stores the new value of SEQ_ASTEP_INC_TIME or SEQ2_ASTEP_INC_TIME into variable seqN.astepIncTime.
					
				*errors+=CHAN_addr(IO24_NUM,seqN->astepIncTimeAddr);						// Switch seqN parameter passed to the function, sets current address at address of SEQ_ASTEP_INC_TIME or SEQ2_ASTEP_INC_TIME.
				*errors+=CHAN_setByte(IO24_NUM,seqN->astepIncTime);							// Writes seqN->astepIncTime at current address.
				if (c!='0') {																//  If the sequencer passed to the function is not Seq.
					printf("Seq%c steps delay Ts%c ",c,c);									//
					printf("is now set such as Ts%c=Tseq*",c);								//
					printf("(SEQ%c_ASTEP_INC_TIME+1)=%dms\n",c,(seqN->astepIncTime+1)*5);	//   Displays new steps delay of Seq2.
				}
				else {																		//  Else
					printf("Seq steps delay Ts is now set such as Ts=Tseq*");				//
					printf("(SEQ_ASTEP_INC_TIME+1)=%dms\n",(seqN->astepIncTime+1)*5);		//   Displays new steps delay of Seq.
				}
			}																				
			if (c2!='r') return displaySeqMenu(c,seqN,errors);								// If charactere typed is not 'r' displays recursively menu of the sequencer passed to the function.
			else printf("return\n");														// Else 
			displayMainMenu();																//  Displays main menu.
		}
	}
	return exit_loop;
}

void invertWrap(sequencers* seqs, uint32_t* errors) { 						// invertWrap() sets SEQ_WRAP to 0 or 1 switch previous value. SEQ_WRAP is common setting to the three sequencers.
	if (seqs->wrap==0) {													// 
		seqs->wrap=1;														//
	}																		//
	else seqs->wrap=0;														// Sets variable seqs.wrap to 0 or 1 switch its previous value.
	*errors+=CHAN_addr(IO24_NUM,seqs->wrapAddr);							// Sets current address at address of SEQ_WRAP.
	*errors+=CHAN_setByte(IO24_NUM,seqs->wrap);								// Writes value of seqs.wrap at current address.
	printf("wrap set to %d\n",seqs->wrap);									// Displays new value of SEQ_WRAP.
	if (seqs->wrap==1) *errors+=CHAN_command(IO24_NUM,7);					// Executes IO24 command 0x07 named STEPCOUNT_SEQUENCER_RUN : restarts stepcount sequenced outputs mode because SEQ_WRAP has been set to 1.
	displayMainMenu();														// Displays main menu.
}

void pauseResumeStep(sequencer* seqN, uint32_t* errors) {   				// pauseResumeStep() pauses/resumes sequencer passed in parameter.
	static uint8_t stepN=0;											
	if (!stepN) {															// If Seq increases or decreases.
		printf("%s paused\n",seqN->name);									//
		stepN=seqN->presetStep;												// 	Saves value of seqN.Step in stepN.
		seqN->presetStep=0;													//  Sets seqN.step to 0.
	}
	else {																	// Else
		printf("%s running\n",seqN->name);									//  
		seqN->presetStep=stepN;												//	Sets seqN.step to saved value of stepN.
		stepN=0;															//  Resets stepN.
	}
	*errors+=CHAN_addr(IO24_NUM,seqN->presetStepAddr);						// Sets current address at address of SEQ_STEP.
	*errors+=CHAN_setByte(IO24_NUM,seqN->presetStep);						// Writes value of seqN.step at current address.
	errors+=CHAN_command(IO24_NUM,7);										// Executes IO24 command 0x07 named STEPCOUNT_SEQUENCER_RUN because SEQ_PRESET_STEP has been edited.
	displayMainMenu();														// Displays main menu.
}

void add10Steps(sequencer* seqN, uint32_t* errors) { 			    		// add10Steps() adds 10 steps to the sequencer passed in parameter. 
	uint8_t countN=0;												 
	*errors+=CHAN_addr(IO24_NUM,seqN->presetCountAddr);						// Sets current address at address of SEQ_PRESET_COUNT.
	CHAN_getBytes(IO24_NUM,1,&countN);										// Stores value of SEQ_PRESET_COUNT in variable countN.
	*errors+=CHAN_addr(IO24_NUM,seqN->presetCountAddr);						// Sets current address at address of SEQ_PRESET_COUNT.
	*errors+=CHAN_setByte(IO24_NUM,seqN->presetCount+countN);				// Writes 10+countN at current address.	
	if (countN==0) {														// If the sequencer has finished its sequence.
		printf("Restarting sequencers for 10 steps.\n");					//
		errors+=CHAN_command(IO24_NUM,7);									// 	Executes IO24 command 0x07 named STEPCOUNT_SEQUENCER_RUN to restart stepcount sequenced outputs mode. 
	}
	else {																	// Else 
		printf("Remaining steps for sequencer %s : %d\n",
			seqN->name,seqN->presetCount+countN);							//  Displays remaining steps for the sequencer.
	}	
	displayMainMenu();														// Displays main menu.
}

void displayStatus(sequencer* seqN, uint32_t* errors) { 					// displayStatus() displays sense and number of remaining steps for the sequencer passed in parameter. 
	uint8_t countN=0;																
	uint8_t stepN=0;
	*errors+=CHAN_addr(IO24_NUM,seqN->presetCountAddr);						// Sets current address at address of SEQ_PRESET_COUNT.			
	CHAN_getBytes(IO24_NUM,1,&countN);										// Stores value of SEQ_PRESET_COUNT in variable countA.
	printf("Remaining step(s) for sequencer %s : %d\n",seqN->name,countN);	//
	*errors+=CHAN_addr(IO24_NUM,seqN->presetStepAddr);						// Sets current address at address of SEQ_PRESET_STEP.
	CHAN_getBytes(IO24_NUM,1,&stepN);										// Stores value of SEQ_PRESET_STEP in variable stepA.
	if (stepN==0) printf("sequencer %s does nothing\n",seqN->name);			//
	else if (stepN==1) printf("sequencer %s increases\n",seqN->name);		//
	else if (stepN==2) printf("sequencer %s decreases\n",seqN->name);		// Displays if Seq does nothing, increases or decreases.
	displayMainMenu();														// Displays main menu.		
}

 /***************************************************************************************************************************************************************************************************************************************************
 *  MAIN PROGRAM 												                                                                                                                                                                                    *
 ***************************************************************************************************************************************************************************************************************************************************/
 
 
int main (int argc,char *argv[ ]) {
	
	bus_struct chantilly_stack;
	uint8_t myTXBuffer[1024]={0}; 					 
	uint8_t exit_loop=0;
	uint8_t c=0;
	uint32_t errors=0;
	
	sequencer seq={"Seq\0",0xFF,0x150,0x04,0x141,30,0x144,10,0x1CC,1,0x1CF};	// The sructure storing specific settings to sequencer Seq.
	//  seq.mask=0xFF;									
	//  seq.maskAddr=0x150;															
	//  seq.cursor=0x04;								
	//  seq.cursorAddr=0x141;							
	//  seq.astepIncTime=30;							
	//  seq.astepIncTimeAddr=0x144;						
	//  seq.presetCount=10;								
	//  seq.presetCountAddr=0x1cc;						
	//  seq.presetStep=1;																	
	//  seq.presetStepAddr=0x1cf;						
	
	sequencer seq1={"Seq1\0",0xF0,0x151};										// The sructure storing specific settings to sequencer Seq1.
	//  seq1.mask=0xF0;															// Only usefull members of seq1 are initialized : seq1.mask and seq1.maskAddr. 
	//  seq1.maskAddr=0x151;													//
	
	sequencers seqs={0,0x1D2,1,0x146};				                          	// The structure storing common settings to the three sequencers.
	//	seqs.stepcountRestart=0;						  
	//	seqs.stepcountRestartAddr=0x1D2;				 
	//  seqs.wrap=1;									
	//	seqs.wrapAddr=0x146;		
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP CHANTILLY																										                                                                            *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	if(CHAN_setup("APP100",1)!=0) { printf("Error opening chantilly , error or processID { }\n"); CHAN_close();return 0;}	// stops here if can't open immediate core access mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											CHECKS CHANTILLY STACK 				                                                                                                                                                                *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	CHAN_checkBUS(&chantilly_stack,1,1);			// Checks what's on the addr1 of the chantilly stack bus.
	CHAN_checkBUS(&chantilly_stack,2,1);			// Checks what's on the addr2 of the chantilly stack bus.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP UPDATE PERIOD OF SEQUENCED OUTPUTS MODE 				                                                                                                                                    *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	errors+=CHAN_addr4(IO24_NUM,0x52);             	// Sets current address to 0x52*4=0x148 into IO24 (CHAN_addr4() can only access multiple of 4 offsets,
													// 													must do empty reading or writing to increase current address). 
	errors+=CHAN_setByte(IO24_NUM,0x08);           	// Writes 0x08 at current address 0x148 into the IO24 (current address increases automatically).
	errors+=CHAN_setByte(IO24_NUM,0xCF);			// Writes 0xCF at current address 0x149 into the IO24.  
													// Two previous values at addresses 0x148 and 0x149 are used to set  SEQ_USER_VAL=0xCF08 => 0xFFFF-SEQ_USER_VAL=0x30F7=12535. 
	errors+=CHAN_setByte(IO24_NUM,0x01);			// Writes 0x01 at current address 0x14A into IO24.
													// Previous value is used to set SEQ_USER_PRESCA=1 => PRESCAseq=1/4 (0,1,2,..,7 respectively corresponding to 1/2,1/4,1/8,..,1/256). 
	// Sequencer mode is now set with update period, Tseq, such as Tseq=(0xFFFF-SEQ_USER_VAL)*100ns/PRESCAseq=12535*100ns*4=5.014ms.
	// That means, inputs are read every Tseq=5.014ms and sequenced outputs are updated every (SEQ_ASTEP_INC_TIME+1)*Tseq=(SEQ_ASTEP_INC_TIME+1)*5.014ms. 

	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP SEQUENCED OUTPUTS MODE				                                                                                                                                                        *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	/** SETS UP SEQUENCED OUTPUTS */			
	
	errors+=CHAN_addr4(IO24_NUM,0x40);				// Sets current address at 0x40*4=0x100 into IO24.
	errors+=CHAN_setByte(IO24_NUM,0x00);			// Writes 0 at current address 0x100 into IO24. Used to set OUTPUT_A=0. 
	errors+=CHAN_setByte(IO24_NUM,0);				// Writes 0 at current address 0x101 into IO24. Used to set OUTPUT_B=0. 
	errors+=CHAN_addr4(IO24_NUM,0x54);				// Sets current address at 0x54*4=0x150 into IO24.
	errors+=CHAN_setByte(IO24_NUM,seq.mask);		// Writes seq.mask(=0xFF) at current address 0x150 into IO24. Used to set SEQ_MASK=0xFF
	errors+=CHAN_setByte(IO24_NUM,seq1.mask);		// Writes seq1.mask(=0xF0) at current address 0x151 into IO24. used to set SEQ1_MASK=0xF0
	// Since mode of sequencer is 0, Seq play on OutputA such as OutputA=data[SEQ_CURSOR]&SEQ_MASK|OUTPUT_A with data[X]:value stored at address X and 0x04≤X≤0xFF.
	//								 Seq plays on OutputB such as OutputB=data[SEQ_CURSOR+128]&SEQ1_MASK|OUTPUT_B.
	// With above settings Seq plays on all bits of OutputA and on all bits of OutputB. 
	
	
	/** WRITES SEQUENCES VALUES */
	
	myTXBuffer[00]=0x11;							// Seq sequence on OutputA			
	myTXBuffer[01]=0x22;							// (1st sequence of the couple)	
	myTXBuffer[02]=0x44;							//  							
	myTXBuffer[03]=0x88;							//						
	errors+=CHAN_addr(IO24_NUM,0x04);				// Sets current address at 0x04 into IO24.
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Writes sequence (via myTXBuffer) at current address 0x04 into IO24.
	// Using mode 0 implies that first sequence of the couple is the sequence of Seq on OutputA. This sequence must be stored between included addresses 0x04 and 0x7F in RAM.
	
	myTXBuffer[00]=0x10;							// Seq sequence on OutputB			
	myTXBuffer[01]=0x20;							// (2nd sequence of the couple)	
	myTXBuffer[02]=0x40;							//  							
	myTXBuffer[03]=0x80;							//								
	errors+=CHAN_addr(IO24_NUM,0x84);				// Sets current address at 0x84 into IO24.
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Writes sequence (via myTXBuffer) at current address 0x04 into IO24.
	// Using mode 0 implies that second sequence of the couple is the sequence of Seq on OutputB. This sequence must be stored between included addresses 0x84 and 0xFF in RAM.
	
	
	/** SETS UP SEQUENCER Seq SETTINGS */
	
													// Presets via myTXBuffer :
	myTXBuffer[0]=0; 		  						// SEQ_STEP is useless in stepcount mode and replaced by SEQ_PRESET_STEP.
	myTXBuffer[1]=seq.cursor;						// SEQ_CURSOR=0x04 : 	 current RAM address of Seq sequence (permanently overwritten but as Seq increase must be initialized such as SEQ_CURSOR≤SEQ_END).
	myTXBuffer[2]=0x04;								// SEQ_START=0x04 : 	 RAM address where (common) Seq sequence begins.
	myTXBuffer[3]=0x07;   							// SEQ_END=0x08 : 		 RAM address where (common) Seq sequence ends. In mode 0, 0x04≤SEQ_START≤SEQ_END≤0x7F.
	myTXBuffer[4]=seq.astepIncTime;					// SEQ_ASTEP_INC_TIME :  used to change delay Ts between steps of Seq sequence such as Ts=(SEQ_ASTEP_INC_TIME+1)*Tseq=31*0.005014=155.434 ms. => fs=6.43Hz. 
	myTXBuffer[5]=0;								/*********************************************non implementé ??????******************************/
	myTXBuffer[6]=1;								// SEQ_WRAP=1 : 		 plays sequences infinitely by setting SEQ_CURSOR=SEQ_START if SEQ_STEP=1 or SEQ_CURSOR=SEQ_END if SEQ_STEP=2 when Seq sequence ends.
	myTXBuffer[7]=0;								// SEQ_MODE=0 : 		 mode 0, only Seq settings are required.
	errors+=CHAN_addr4(IO24_NUM,0x50);				// Sets current address at 0x50*4=0x140 into IO24.
	errors+=CHAN_setBytes(IO24_NUM,8,myTXBuffer);	// Writes sequencer Seq settings (stored in myTXBuffer) at current address 0x140 in RAM.
	errors+=CHAN_addr(IO24_NUM,seq.presetCountAddr);// Sets current address at 0x1CC into IO24.
	errors+=CHAN_setByte(IO24_NUM,seq.presetCount);	// Sets SEQ_PRESET_COUNT=10 : sequencer Seq executes 10 steps.
	errors+=CHAN_addr(IO24_NUM,seq.presetStepAddr);	// Sets current address at 0x1CF into IO24.
	errors+=CHAN_setByte(IO24_NUM,seq.presetStep);	// Sets SEQ_PRESET_STEP=1 : sequencer Seq increases.
	errors+=CHAN_addr(IO24_NUM,seqs.stepcountRestartAddr);	// Sets current address at 0x1D2 into IO24.
	errors+=CHAN_setByte(IO24_NUM,seqs.stepcountRestart);	// Sets SEQS_STEPCOUNT_RESTART=1 : sets SEQ_CURSOR=SEQ1_CURSOR=SEQ_START when executing the IO24 command named STEPCOUNT_SEQUENCER_RUN.
	// To edit SEQ_MODE and SEQ_WRAP, sequencer mode must be restarted by executing IO24 command 0x07 named STEPCOUNT_SEQUENCER_RUN. 
	// All others sequencer settings can be edited when stepcount sequenced outputs mode is running but it is not advisable.
			
	
	/************************************************************************************************************************************************************************************************************************************************
	*											STARTS SEQUENCED OUTPUTS MODE																																										*
	************************************************************************************************************************************************************************************************************************************************/											
	
	
	errors+=CHAN_command(IO24_NUM,0x07);			// Executes IO24 command 0x07 named STEPCOUNT_SEQUENCER_RUN : Starts (immediate inputs and) stepcount sequenced outputs mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											LOOPS :			             	                                                                                                                                                                    *
	************************************************************************************************************************************************************************************************************************************************/


	printf("\nIO24 stepcount sequenced outputs mode 0 is started : \n");
	printf("Sequencers Seq is running with steps delay Ts set such as ");
	printf("Ts=Tseq*(SEQ_ASTEP_INC_TIME+1)");
	printf("with SEQ_ASTEP_INC_TIME=30 and Tseq=5ms ");
	printf("then Ts=31*5ms=155ms\n\n");
	displayMainMenu();
	
	while (!exit_loop) {														// Entering loop
		getch_start();															//
		c=getch_try();															//
		getch_stop();															//  Gets the last character typed without stopping program (see FUNCTIONS above).
		if (c) {                                                    			//  
			if (c=='q') {														//
				printf("\n");													//
				exit_loop=1;													//   If 'q' is typed exits loop.
			}																	//
			else if (c=='w') invertWrap(&seqs,&errors);							//	 Else if 'w' is typed inverts wrap of the three sequencers (even if Seq2 is unused). . 
			else if (c=='p') pauseResumeStep(&seq,&errors);						//	 Else if 'p' is typed pauses/resumes the two sequencers Seq and Seq1. 
			else if (c=='a') add10Steps(&seq,&errors);							//	 Else if 'a' is typed adds 10 steps to the two sequencers Seq and Seq1. 
			else if (c=='?') displayStatus(&seq,&errors);						// 	 Else if '?' is typed displays status
			else if (c=='0') exit_loop=displaySeqMenu(c,&seq,&errors); 			// 	 Else if '0' is typed shows Seq menu.																											
		}
	}
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											EXITS PROGRAM				                                                                                                                                                                        *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	/** STOPS IO24 */
	
	errors+=CHAN_command(IO24_NUM,6);           	// Executes IO24 command 0x06 named STOP : Stops IO24 running mode (optionnal, if not executed IO24 will go on maintening mode
													//																				actually running after having closed chantilly and program).  
	
	/** CLOSES CHANTILLY	*/
	
	errors+=CHAN_close();							// Closes immediate core access mode.
	
	
	/** DEBUG */
	
	printf("\naccess errors : %d\n\n",errors);		// Displays access errors (optionnal for debug).
	fflush(stdout);
	
	return 0;
	
}
