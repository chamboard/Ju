 /*******************************************************************
 *                                                                  * 
 * Chantilly 0x10 - IO24 : Source code for application note AN112   *
 *                                                     			    *
 ****************************************************************************************************************************************************************************************************************************************************
 * CIRCUIT DESCRIPTION :																																					                                                                        *
 * Core module IO24 is set at number 1 on chantilly stack (redefine IO24_NUM according to hardware-address to change it : between 1 included and 6 included).																						*
 * As described in file chantilly_0x10_AN112_pcb2rev1.PDF, a bar led is wired on OutputA and OutputB.																																				*
 * 																																																													*
 *  																																																												* 
 * PROGRAM DESCRIPTION : Test using IO24 direct core access mode with (immediate inputs and) sequenced outputs in mode 2.                                                                    		                                               	*
 * One common sequence is written in free RAM.																																																		* 
 * Due to mode 2, sequencer Seq play the sequence on OutputA and sequencer Seq2 plays the same sequence on OutputB.  																																*
 * SEQ_MASK and SEQ2_mask initial values make Seq outputted on all bits of OutputA and Seq2 outputted on all bits of OutputB.																														*
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
 
typedef struct															// Structure storing SEQ_WRAP and its address.
{																		// SEQ_WRAP is a common setting to the sequencers Seq and Seq2 in mode2.
	uint8_t wrap;
	uint32_t wrapAddr;
}
sequencers;                                                             

typedef struct															// Structure storing specific settings to each sequencer.
{	
	uint8_t mask;
	uint32_t maskAddr;
	uint8_t step;
	uint8_t stepAddr4;
	uint8_t cursor;	
	uint32_t cursorAddr;
	uint8_t astepIncTime;
	uint32_t astepIncTimeAddr;
	uint8_t unfreezeResync;
	uint32_t unfreezeResyncAddr;
	uint8_t savedCursor;
	uint32_t savedCursorAddr;
	uint8_t freezeCount;
	uint32_t freezeCountAddr;
	uint8_t frozen;
	uint8_t freezeMask;
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


void initCursor(sequencer* seqN,uint32_t* errors) {						// initCursor() initializes cursor of the sequencer passed in parameter.
	if (seqN->step==1) seqN->cursor=0x04;								// Using mode 2 implies Seq or Seq2 can be passed in parameter.
	else seqN->cursor=0x07;
	*errors+=CHAN_addr(IO24_NUM,seqN->cursorAddr);
	*errors+=CHAN_setByte(IO24_NUM,seqN->cursor);
}

void displayMainMenu() {
	printf("\nMain menu\n");
	printf("Type :\t'0' to display Sequencer Seq menu.\n");
	printf("\t'2' to display Sequencer Seq2 menu.\n");
	printf("\t'w' to invert wrap.\n");
	printf("\t'p' to pause/resume.\n");
	printf("\t'q' to exit.\n");
}

/*uint32_t displaySeqMenu(char c,sequencer* seqN,uint8_t* exit_loop) { 						// displaySeqMenu() lets the user edit specific settings of the sequencer passed in parameterr.
	uint8_t c2='\0';																		// Using mode 1 implies Seq or seq2 can be passed in parameter.
	uint32_t errors=0;
	uint8_t buf[8];
	static uint8_t frozenA=0; 
	static uint8_t frozenB=0;
	if (c!='0') printf("\nSequencer Seq%c menu\n",c);
	else printf("\nSequencer Seq menu\n");
	printf("\tType\t:\t'm' to edit mask.\n");
	printf("\t\t\t's' to invert step.\n");
	printf("\t\t\t'd' to change steps delay.\n");
	printf("\t\t\t'f' to freeze/release.\n");
	printf("\t\t\t'g' to freeze & force/release.\n");
	printf("\t\t\t'h' to synchronize.\n");
	printf("\t\t\t'r' to return.\n");
	printf("\t\t\t'q' to quit.\n");
	fflush(stdout);
	while (c2!='q'&&c2!='m'&&c2!='d'&&c2!='r'&&c2!='s'&&c2!='f'&&c2!='g'&&c2!='h') {
		getch_start();																		//
		c2=getch_try();																		//
		getch_stop();																		// Gets the last character typed without stopping program (see FUNCTIONS above).
	}
	if (c2) {
		if (c2=='q') *exit_loop=1;															// If character typed is 'q' : Exits loop.
		else {
			if (c2=='m') {																	// If character typed is 'm' : Edits the mask.
				if (c!='0') {
					printf("\nSEQ%c_MASK = 0x%02X\t",c,seqN->mask);
					printf("Enter new value for SEQ%c_MASK = 0x",c);
				}
				else {
					printf("\nSEQ_MASK = 0x%02X\t",seqN->mask);
					printf("Enter new value for SEQ_MASK = 0x");
				}			
				scanf("%2x",(unsigned int*)&seqN->mask);	
				errors+=CHAN_addr(IO24_NUM,seqN->maskAddr);									// Switch seqN parameter passed to the function, sets current address at address of SEQ_MASK or SEQ2_MASK into IO24.
				errors+=CHAN_setByte(IO24_NUM,seqN->mask);									// Writes seqN.mask at current address.	
			}
			else if (c2=='s') {																// If character typed is 's' : Edits the sense.
				if (seqN->step==1) seqN->step=2;
				else if (seqN->step==2) seqN->step=0;
				else seqN->step=1;
				if (c!='0') { 																//  If the sequencer passed to the function is not Seq.
					errors+=CHAN_addr4(IO24_NUM,seqN->stepAddr4);							//   Sets current address at address of SEQ2_STEP into IO24.
					errors+=CHAN_setByte(IO24_NUM,seqN->step);								//   Writes seqN.step at current address.
					if (seqN->step==0) printf("sequencer Seq%c does nothing\n",c);			//
					else if (seqN->step==1) printf("sequencer Seq%c increases\n",c);		//
					else if (seqN->step==2) printf("sequencer Seq%c decreases\n",c);		//   Displays the new sense of the sequencer Seq2.
				}
				else {																		//  Else
					errors+=CHAN_addr4(IO24_NUM,seqN->stepAddr4);							//   Sets current address at address of SEQ_STEP into IO24.
					errors+=CHAN_setByte(IO24_NUM,seqN->step);								//   Writes seqN.step at current address.
					if (seqN->step==0) printf("sequencer Seq does nothing\n");				//
					else if (seqN->step==1) printf("sequencer Seq increases\n");			//
					else if (seqN->step==2) printf("sequencer Seq decreases\n");			//   Displays the new sense of the sequencer Seq.
				}		
			}
			else if (c2=='d') {																// If character typed is 'd' : Edits delay between sequence steps. 
				errors+=CHAN_addr(IO24_NUM,0x1CB);											//  Sets current address at address of SEQS_FREEZE_MASK.
				if (c!='0') {																//  If the sequencer passed to the function is not Seq.
					errors+=CHAN_setByte(IO24_NUM,4);										//   Writes 4 at current address to define Seq2 as sequencer to freeze.
					printf("Seq%c steps delay Ts%c is such as",c,c);						//   Displays SEQ2_ASTEP_INC_TIME and current steps delay of the sequencer Seq2.
					printf(" Ts%c=Tseqs*(SEQ%c_ASTEP_INC_TIME+1)",c,c);						//
					printf(" with Tseqs=5ms and ");											//
					printf("SEQ%c_ASTEP_INC_TIME=%d\n",c,seqN->astepIncTime);				//
					printf("Enter new value for SEQ%c_ASTEP_INC_TIME=",c);					//   Asks a new value for SEQ2_ASTEP_INC_TIME.
					scanf("%3d",(unsigned int*)&seqN->astepIncTime);						//   Stores the new value of SEQ2_ASTEP_INC_TIME into variable seqN.astepIncTime.
					CHAN_command(IO24_NUM,2);												//   Executes IO24 command 0x02 named FREEZE. Freezes the sequencers defined by SEQS_FREEZE_MASK.
					errors+=CHAN_addr(IO24_NUM,seqN->astepIncTimeAddr);						//   Sets current address at address of SEQ2_ASTEP_INC_TIME.
					errors+=CHAN_setByte(IO24_NUM,seqN->astepIncTime);						//   Writes seqN.astepIncTime at current address.
					printf("Seq%c steps delay Ts%c ",c,c);									//
					printf("is now set such as Ts%c=Tseqs*",c);								//
					printf("(SEQ%c_ASTEP_INC_TIME+1)=%dms\n",c,(seqN->astepIncTime+1)*5);	//   Displays new steps delay of Seq2.
				}                                   
				else {																		//  Else
					errors+=CHAN_setByte(IO24_NUM,1);										// 	 Writes 1 at current address to define Seq as sequencer to freeze.			         										
					printf("Seq steps delay Ts is such as Ts=Tseqs*(SEQ_ASTEP_INC_TIME+1)");//   Displays SEQ_ASTEP_INC_TIME and current steps delay of the sequencer Seq. 
					printf(" with Tseqs=5ms and SEQ_ASTEP_INC_TIME=%d\n",seqN->astepIncTime);   
					printf("Enter new value for SEQ_ASTEP_INC_TIME=");						//   Asks a new value for SEQ_ASTEP_INC_TIME.  
					scanf("%3d",(unsigned int*)&seqN->astepIncTime);						//   Stores the new value of SEQ_ASTEP_INC_TIME into variable seqN.astepIncTime.
					CHAN_command(IO24_NUM,2);												//   Executes IO24 command 0x02 named FREEZE. Freezes the sequencers defined by SEQS_FREEZE_MASK. 
					errors+=CHAN_addr(IO24_NUM,seqN->astepIncTimeAddr);						//   Sets current address at address of SEQ_ASTEP_INC_TIME.
					errors+=CHAN_setByte(IO24_NUM,seqN->astepIncTime);						//   Writes seqN.astepIncTime at current address. 
					printf("Seq steps delay Ts is now set such as Ts=Tseqs*");				//
					printf("(SEQ_ASTEP_INC_TIME+1)=%dms\n",(seqN->astepIncTime+1)*5);		//   Displays new steps delay of Seq.
				}	
				errors+=CHAN_addr(IO24_NUM,0x14B);											//    Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
				errors+=CHAN_setByte(IO24_NUM,0);											//    Writes 0 at current address : To synchronize Seq2 with any step of Seq.
				CHAN_command(IO24_NUM,4);													//    Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
			}
			else if (c2=='f') {																// If character typed is 'f' : Freezes/releases sequencer.
				errors+=CHAN_addr(IO24_NUM,0x1CB);											//  Sets current address at address of SEQS_FREEZE_MASK.
				if (c!='0') {																//  If the sequencer passed to the function is not Seq.
					if (!frozenA) {															//   If Seq2 is not frozen
						errors+=CHAN_setByte(IO24_NUM,4);									//    Writes 4 at current address to define Seq2 as sequencer to freeze.
						CHAN_command(IO24_NUM,2);											//    Executes IO24 command 0x02 named FREEZE : The sequencers defined by SEQS_FREEZE_MASK are frozen.
						frozenA=1;
						printf("Sequencer Seq2 is frozen.\n");
						fflush(stdout);
					}
					else {																	//   Else
						errors+=CHAN_setByte(IO24_NUM,4);									//    Writes 4 at current address to define Seq2 as sequencer to release.
						errors+=CHAN_addr(IO24_NUM,0x14B);									//    Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
						errors+=CHAN_setByte(IO24_NUM,0);									//    Writes 0 at current address : To synchronize Seq2 with any step of Seq.
						CHAN_command(IO24_NUM,4);											//    Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
						frozenA=0;
						printf("Sequencer Seq2 is released.\n");							//
						errors+=CHAN_addr(IO24_NUM,0x1CA);									//
						errors+=CHAN_getBytes(IO24_NUM,1,buf);								//
						printf("It has been frozen during\tTf=SEQ2_FREEZE_COUNT*Ts1");		//
						printf(" with SEQ2_FREEZE_COUNT=%d\n",buf[0]); 						//
						printf("\t\t\t\t");													//
						printf("Tf=SEQ2_FREEZE_COUNT*(SEQ2_ASTEP_INC_TIME+1)*Tseqs\n");		//
						printf("\t\t\t\t");													//
						printf("Tf=%d*(%d+1)*5ms\n",buf[0],seqN->astepIncTime);				//
						printf("\t\t\t\t");													//
						printf("Tf=%dms\n",buf[0]*(seqN->astepIncTime+1)*5);				//    Displays SEQ2_FREEZE_COUNT and freeze time values.
						fflush(stdout);														//
					}
				}
				else {																		//  Else
					if (!frozenB) {															//   If Seq is not frozen.
						errors+=CHAN_setByte(IO24_NUM,1);									//    Writes 1 at current address to define Seq as sequencer to freeze.
						CHAN_command(IO24_NUM,2);											//    Executes IO24 command 0x02 named FREEZE : The sequencers defined by SEQS_FREEZE_MASK are frozen.
						frozenB=1;
						printf("Sequencer Seq is frozen.\n");
						fflush(stdout);
					}
					else {																	//   Else
						errors+=CHAN_setByte(IO24_NUM,1);									//	  Writes 1 at current address to define Seq as sequencer to release.	
						errors+=CHAN_addr(IO24_NUM,0x14B);									//    Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
						errors+=CHAN_setByte(IO24_NUM,0);									//    Writes 0 at current address : To synchronize Seq with any step of Seq2.	 
						CHAN_command(IO24_NUM,4);											//    Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
						frozenB=0;
						printf("Sequencer Seq is released.\n");								//
						errors+=CHAN_addr(IO24_NUM,0x1C8);									//
						errors+=CHAN_getBytes(IO24_NUM,1,buf);								//
						printf("It has been frozen during\tTf=SEQ_FREEZE_COUNT*Ts");		//
						printf(" with SEQ_FREEZE_COUNT=%d\n",buf[0]); 						//
						printf("\t\t\t\t");													//
						printf("Tf=SEQ_FREEZE_COUNT*(SEQ_ASTEP_INC_TIME+1)*Tseqs\n");		//
						printf("\t\t\t\t");													//
						printf("Tf=%d*(%d+1)*5ms\n",buf[0],seqN->astepIncTime);				//
						printf("\t\t\t\t");													//
						printf("Tf=%dms\n",buf[0]*(seqN->astepIncTime+1)*5);				//    Displays SEQ_FREEZE_COUNT and freeze time values.
						fflush(stdout);														//
					}
				}
			}
			else if (c2=='g') {																// If character typed is 'g' : Freezes & forces/releases sequencer.
				errors+=CHAN_addr(IO24_NUM,0x1CB);											//  Sets current address at address of SEQS_FREEZE_MASK.
				if (c!='0') {																//  If the sequencer passed to the function is not Seq.
					if (!frozenA) {															//   If Seq2 is not frozen
						errors+=CHAN_setByte(IO24_NUM,4);									//    Writes 2 at current address to define Seq2 as sequencer to freeze.
						CHAN_command(IO24_NUM,2);											//    Executes IO24 command 0x02 named FREEZE : The sequencers defined by SEQS_FREEZE_MASK are frozen.
						errors+=CHAN_addr(IO24_NUM,0x02);									//    Sets current address at address of SEQ2_SAVED_CURSOR.
						errors+=CHAN_setByte(IO24_NUM,seqN->mask);							//    Writes seqN.mask at current address to force all bits defined by SEQ2_MASK to 1 on OutputA.
						frozenA=1;
						printf("Sequencer Seq2 is frozen.\n");
						fflush(stdout);
					}
					else {																	//   Else
						errors+=CHAN_setByte(IO24_NUM,4);									//    Writes 2 at current address to define Seq2 as sequencer to release.
						errors+=CHAN_addr(IO24_NUM,0x14B);									//    Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
						errors+=CHAN_setByte(IO24_NUM,0);									//    Writes 0 at current address : To synchronize Seq2 with any step of Seq.
						CHAN_command(IO24_NUM,4);											//    Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
						frozenA=0;
						printf("Sequencer Seq2 is released.\n");
						fflush(stdout);
					}
				}
				else {																		//  Else
					if (!frozenB) {															//   If Seq is not frozen.
						errors+=CHAN_setByte(IO24_NUM,1);									//    Writes 1 at current address to define Seq as sequencer to freeze.
						CHAN_command(IO24_NUM,2);											//    Executes IO24 command 0x02 named FREEZE : The sequencers defined by SEQS_FREEZE_MASK are frozen.
						errors+=CHAN_addr(IO24_NUM,0x00);									//    Sets current address at address of SEQ_SAVED_CURSOR.
						errors+=CHAN_setByte(IO24_NUM,seqN->mask);							//    Writes seqN.mask at current address to force all bits defined by SEQ_MASK to 1 on OutputB.
						frozenB=1;
						printf("Sequencer Seq is frozen.\n");
						fflush(stdout);
					}
					else {																	//  Else
						errors+=CHAN_setByte(IO24_NUM,1);									//	 Writes 1 at current address to define Seq as sequencer to release.	
						errors+=CHAN_addr(IO24_NUM,0x14B);									//   Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
						errors+=CHAN_setByte(IO24_NUM,0);									//   Writes 0 at current address : To synchronize Seq with any step of Seq2.	 
						CHAN_command(IO24_NUM,4);											//   Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
						frozenB=0;
						printf("Sequencer Seq is released.\n");
						fflush(stdout);
					}
				}
			}
			else if (c2=='h') {																// If character typed is 'h' : Synchronizes sequencer.
				errors+=CHAN_addr(IO24_NUM,0x1CB);											//  Sets current address at address of SEQS_FREEZE_MASK.
				if (c!='0') {																//  If the sequencer passed to the function is not Seq.
					errors+=CHAN_setByte(IO24_NUM,4);										//   Writes 2 at current address to define Seq2 as sequencer to freeze and resync.
					errors+=CHAN_addr(IO24_NUM,0x14B);										//   Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
					errors+=CHAN_setByte(IO24_NUM,4);										//   Writes 4 at current address : To synchronize Seq2 with the first step of Seq (when SEQ_CURSOR=4). 
					if (seqN->step==1) seqN->cursor=4;										//   If Seq2 increases presets its cursor at the start of the sequence.
					else if (seqN->step==2) seqN->cursor=7;									//   If Seq2 decreases presets its cursor at the end of the sequence.
					CHAN_command(IO24_NUM,2);												//   Executes IO24 command 0x02 named FREEZE. Freezes the sequencers defined by SEQS_FREEZE_MASK.
					errors+=CHAN_addr(IO24_NUM,seqN->cursorAddr);							//   Sets current address at address of SEQ2_CURSOR.
					errors+=CHAN_setByte(IO24_NUM,seqN->cursor);							//   Writes new value of SEQ2_CURSOR
					CHAN_command(IO24_NUM,4);												//   Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
					printf("Sequencer Seq2 is synchronised with sequencer Seq as reference.\n");
					fflush(stdout);
				}
				else {
					errors+=CHAN_setByte(IO24_NUM,1);										//   Writes 1 at current address to define Seq as sequencer to freeze and resync.
					errors+=CHAN_addr(IO24_NUM,0x14B);										//   Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
					errors+=CHAN_setByte(IO24_NUM,4);										//   Writes 4 at current address : To synchronize Seq with the first step of Seq2 (when SEQ2_CURSOR=4).
					if (seqN->step==1) seqN->cursor=4;										//   If Seq increases presets its cursor at the start of the sequence.
					else if (seqN->step==2) seqN->cursor=7;									//   If Seq decreases presets its cursor at the end of the sequence.
					CHAN_command(IO24_NUM,2);												//   Executes IO24 command 0x02 named FREEZE. Freezes the sequencers defined by SEQS_FREEZE_MASK.
					errors+=CHAN_addr(IO24_NUM,seqN->cursorAddr);							//   Sets current address at address of SEQ_CURSOR.
					errors+=CHAN_setByte(IO24_NUM,seqN->cursor);							//   Writes new value of SEQ_CURSOR
					CHAN_command(IO24_NUM,4);												//   Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
					printf("Sequencer Seq is synchronised with sequencer Seq2 as reference.\n");
					fflush(stdout);
				}
			}
			
			if (c2!='r') return displaySeqMenu(c,seqN,exit_loop);							// If charactere typed is not 'r' displays recursively menu of the sequencer passed to the function.
			else printf("return\n");														// Else 
			displayMainMenu();																//  Displays main menu.
		}
	}
	return errors;
}*/

uint32_t displaySeqMenu(char c,sequencer* seqN,uint8_t* exit_loop) { 						// displaySeqMenu() lets the user edit specific settings of the sequencer passed in parameterr.
	uint8_t c2='\0';																		// Using mode 2 implies Seq or Seq2 can be passed in parameter.
	uint32_t errors=0;
	uint8_t buf[8];
	if (c=='0') c=0;
	printf("\nSequencer Seq%c menu\n",c);
	printf("\tType\t:\t'm' to edit mask.\n");
	printf("\t\t\t's' to invert step.\n");
	printf("\t\t\t'd' to change steps delay.\n");
	printf("\t\t\t'f' to freeze/release.\n");
	printf("\t\t\t'g' to freeze & force/release.\n");
	printf("\t\t\t'h' to synchronize.\n");
	printf("\t\t\t'r' to return.\n");
	printf("\t\t\t'q' to quit.\n");
	fflush(stdout);
	while (c2!='q'&&c2!='m'&&c2!='d'&&c2!='r'&&c2!='s'&&c2!='f'&&c2!='g'&&c2!='h') {
		getch_start();																		//
		c2=getch_try();																		//
		getch_stop();																		// Gets the last character typed without stopping program (see FUNCTIONS above).
	}
	if (c2) {
		if (c2=='q') *exit_loop=1;															// If character typed is 'q' : Exits loop.
		else {
			if (c2=='m') {																	// If character typed is 'm' : Edits the mask.
				printf("\nSEQ%c_MASK = 0x%02X\t",c,seqN->mask);
				printf("Enter new value for SEQ%c_MASK = 0x",c);
				scanf("%2x",(unsigned int*)&seqN->mask);	
				errors+=CHAN_addr(IO24_NUM,seqN->maskAddr);									//  Switch the parameter seqN passed to the function, sets current address at address of SEQ_MASK or SEQ2_MASK into IO24.
				errors+=CHAN_setByte(IO24_NUM,seqN->mask);									//  Writes seqN.mask at current address.	
			}
			else if (c2=='s') {																// If character typed is 's' : Edits the sense.
				if (seqN->step==1) seqN->step=2;
				else if (seqN->step==2) seqN->step=0;
				else seqN->step=1;
				errors+=CHAN_addr4(IO24_NUM,seqN->stepAddr4);								//   Switch the parameter seqN passed to the function, sets current address at address of SEQ_STEP or SEQ2_STEP.
				errors+=CHAN_setByte(IO24_NUM,seqN->step);									//   Writes seqN.step at current address.
				if (seqN->step==0) printf("sequencer Seq%c does nothing\n",c);				//
				else if (seqN->step==1) printf("sequencer Seq%c increases\n",c);			//
				else if (seqN->step==2) printf("sequencer Seq%c decreases\n",c);			//   Switch the parameter seqN passed to the function, displays the new sense of the sequencer Seq or Seq2. 
			}
			else if (c2=='d') {																// If character typed is 'd' : Edits delay between steps of the sequencer synchronizing it with its time reference.   
				errors+=CHAN_addr(IO24_NUM,0x1CB);											//  Sets current address at address of SEQS_FREEZE_MASK.
				errors+=CHAN_setByte(IO24_NUM,seqN->freezeMask);							//   Writes seqN.freezeMask at current address to define the sequencer to freeze/release.
				printf("Seq%c steps delay Ts%c is such as",c,c);							//   Displays SEQ_ASTEP_INC_TIME or SEQ2_ASTEP_INC_TIME and current steps delay of the sequencer Seq or Seq2.
				printf(" Ts%c=Tseqs*(SEQ%c_ASTEP_INC_TIME+1)",c,c);							//
				printf(" with Tseqs=5ms and ");												//
				printf("SEQ%c_ASTEP_INC_TIME=%d\n",c,seqN->astepIncTime);					//
				printf("Enter new value for SEQ%c_ASTEP_INC_TIME=",c);						//   Asks a new value for SEQ_ASTEP_INC_TIME or SEQ2_ASTEP_INC_TIME.
				scanf("%3d",(unsigned int*)&seqN->astepIncTime);							//   Stores the new value of SEQ_ASTEP_INC_TIME or SEQ2_ASTEP_INC_TIME into variable seqN.astepIncTime.
				CHAN_command(IO24_NUM,2);													//   Executes IO24 command 0x02 named FREEZE. Freezes the sequencers defined by SEQS_FREEZE_MASK.
				errors+=CHAN_addr(IO24_NUM,seqN->astepIncTimeAddr);							//   Sets current address at address of SEQ_ASTEP_INC_TIME or SEQ2_ASTEP_INC_TIME.
				errors+=CHAN_setByte(IO24_NUM,seqN->astepIncTime);							//   Writes seqN.astepIncTime at current address.
				printf("Seq%c steps delay Ts%c ",c,c);										//
				printf("is now set such as Ts%c=Tseqs*",c);									//
				printf("(SEQ%c_ASTEP_INC_TIME+1)=%dms\n",c,(seqN->astepIncTime+1)*5);		//   Switch the sequencer passed to the function, displays new steps delay of Seq or Seq2.
				errors+=CHAN_addr(IO24_NUM,0x14B);											//    Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
				errors+=CHAN_setByte(IO24_NUM,0);											//    Writes 0 at current address : To synchronize the frozen sequencer with any step of its time reference.
				CHAN_command(IO24_NUM,4);													//    Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
			}
			else if (c2=='f') {																// If character typed is 'f' : Freezes/releases sequencer.
				errors+=CHAN_addr(IO24_NUM,0x1CB);											//  Sets current address at address of SEQS_FREEZE_MASK.
				errors+=CHAN_setByte(IO24_NUM,seqN->freezeMask);							//   Writes seqN.freezeMask at current address to define the sequencer to freeze/release.
				if (!seqN->frozen){
					CHAN_command(IO24_NUM,2);												//    Executes IO24 command 0x02 named FREEZE : The sequencers defined by SEQS_FREEZE_MASK are frozen.
					seqN->frozen=1;
					printf("Sequencer Seq%c is frozen.\n",c);								//
				}
				else {
					errors+=CHAN_addr(IO24_NUM,0x14B);										//    Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
					errors+=CHAN_setByte(IO24_NUM,0);										//    Writes 0 at current address : To synchronize frozen sequencer with any step of its time reference.
					CHAN_command(IO24_NUM,4);												//    Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
					seqN->frozen=0;
					printf("Sequencer Seq%c is released.\n",c);								//
					errors+=CHAN_addr(IO24_NUM,seqN->freezeCountAddr);						//
					errors+=CHAN_getBytes(IO24_NUM,1,buf);									//
					printf("It has been frozen during\tTf%c=SEQ%c_FREEZE_COUNT*Ts%c",c,c,c);//
					printf(" with SEQ%c_FREEZE_COUNT=%d\n",c,buf[0]); 						//
					printf("\t\t\t\t");														//
					printf("Tf%c=SEQ%c_FREEZE_COUNT*(SEQ%c_ASTEP_INC_TIME+1)*Tseqs\n",c,c,c);//
					printf("\t\t\t\t");														//
					printf("Tf%c=%d*(%d+1)*5ms\n",c,buf[0],seqN->astepIncTime);				//
					printf("\t\t\t\t");														//
					printf("Tf%c=%dms\n",c,buf[0]*(seqN->astepIncTime+1)*5);				//    Switch the sequencer passed to the function, displays SEQ_FREEZE_COUNT or SEQ2_FREEZE_COUNT and frozen time values.
					fflush(stdout);
				}
			}
			else if (c2=='g') {																// If character typed is 'g' : Freezes & forces/releases sequencer.
				errors+=CHAN_addr(IO24_NUM,0x1CB);											//  Sets current address at address of SEQS_FREEZE_MASK.
				errors+=CHAN_setByte(IO24_NUM,seqN->freezeMask);							//   Writes seqN.freezeMask at current address to define the sequencer to freeze & force/release.
				if (!seqN->frozen){
					CHAN_command(IO24_NUM,2);												//    Executes IO24 command 0x02 named FREEZE : The sequencers defined by SEQS_FREEZE_MASK are frozen.
					errors+=CHAN_addr(IO24_NUM,seqN->savedCursorAddr);						//    Sets current address at address of SEQ_SAVED_CURSOR or SEQ2_SAVED_CURSOR.
					errors+=CHAN_setByte(IO24_NUM,seqN->mask);								//    Writes seqN.mask at current address to force all bits defined by seqN.mask to 1 on OutputA for Seq or OutputB for Seq2. 
					seqN->frozen=1;
					printf("Sequencer Seq%c is frozen.\n",c);								//
				}
				else {
					errors+=CHAN_addr(IO24_NUM,0x14B);										//    Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
					errors+=CHAN_setByte(IO24_NUM,0);										//    Writes 0 at current address : To synchronize frozen sequencer with any step of its time reference.
					CHAN_command(IO24_NUM,4);												//    Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
					seqN->frozen=0;
					printf("Sequencer Seq%c is released.\n",c);								//
				}
			}
			else if (c2=='h') {																// If character typed is 'h' : synchronizes sequencer with first step of its time reference.
				errors+=CHAN_addr(IO24_NUM,0x1CB);											//  Sets current address at address of SEQS_FREEZE_MASK.
				errors+=CHAN_setByte(IO24_NUM,seqN->freezeMask);							//   Writes seqN.freezeMask at current address to define the sequencer to freeze/release.
				errors+=CHAN_addr(IO24_NUM,0x14B);											//   Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
				errors+=CHAN_setByte(IO24_NUM,4);											//   Writes 4 at current address : To synchronize the sequencer with the first step of its time reference (when the cursor of the reference equals 4). 
				if (seqN->step==1) seqN->cursor=4;											//   If the sequencer increases presets its cursor at the start of the sequence.
				else if (seqN->step==2) seqN->cursor=7;										//   If the sequencer decreases presets its cursor at the end of the sequence.
				CHAN_command(IO24_NUM,2);													//   Executes IO24 command 0x02 named FREEZE. Freezes the sequencers defined by SEQS_FREEZE_MASK.
				errors+=CHAN_addr(IO24_NUM,seqN->cursorAddr);								//   Sets current address at address of SEQ_CURSOR or SEQ2_CURSOR.
				errors+=CHAN_setByte(IO24_NUM,seqN->cursor);								//   Writes new value of SEQ_CURSOR or SEQ2_CURSOR switch parameter of the function.
				CHAN_command(IO24_NUM,4);													//   Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
				printf("Sequencer Seq%c is synchronised with first step of sequencer Seq%d.\n",c,seqN->unfreezeResync);
				fflush(stdout);
			}	
			if (c2!='r') return displaySeqMenu(c,seqN,exit_loop);							// If charactere typed is not 'r' displays recursively menu of the sequencer passed to the function.
			else printf("return\n");														// Else 
			displayMainMenu();																//  Displays main menu.
		}
	}
	return errors;
}

uint32_t invertWrap(sequencers* seqs) { 									// invertWrap() sets SEQ_WRAP to 0 or 1 switch previous value. SEQ_WRAP is common setting to the three sequencers.
	uint32_t errors=0;
	if (seqs->wrap==0) {													//
		seqs->wrap=1;														//
	}																		//
	else seqs->wrap=0;														// Sets variable seqs.wrap to 0 or 1 switch its previous value.
	errors+=CHAN_addr(IO24_NUM,seqs->wrapAddr);								// Sets current address at address of SEQ_WRAP.
	errors+=CHAN_setByte(IO24_NUM,seqs->wrap);								// Writes value of seqs.wrap at current address.
	printf("wrap set to %d\n",seqs->wrap);
	displayMainMenu();
	return errors;
}

uint32_t pauseResumeSteps(sequencer* seqA, sequencer* seqB) { 				// pauseResumeSteps() pauses/resumes the two sequencers passed in parameters.
	uint32_t errors=0;
	static uint8_t stepA=0,stepB=0;											 
	if (!stepA) {															// If Seq increases or decreases.
		printf("paused\n");													//
		stepA=seqA->step;													// 	Saves value of seqA.step in stepA.
		seqA->step=0;														//  Sets seqA.step to 0.
		stepB=seqB->step;													//  Saves value of seqB.step in stepB.
		seqB->step=0;														//  Sets seqB.step to 0.
	}
	else {																	// Else
		printf("running\n");												//  
		seqA->step=stepA;													//	Sets seqA.step to saved value of stepA.
		stepA=0;															//  Resets stepA.
		seqB->step=stepB;													//	Sets seqB.step to saved value of stepB.
		stepB=0;															//  Resets stepB.
	}
	errors+=CHAN_addr4(IO24_NUM,seqA->stepAddr4);							// Sets current address at address of SEQ_STEP.
	errors+=CHAN_setByte(IO24_NUM,seqA->step);								// Writes value of seqA.step at current address.
	errors+=CHAN_addr4(IO24_NUM,seqB->stepAddr4);							// Sets current address at address of SEQ2__STEP.
	errors+=CHAN_setByte(IO24_NUM,seqB->step);								// Writes value of seqB.step at current address.
	displayMainMenu();
	return errors;
}


 /***************************************************************************************************************************************************************************************************************************************************
 *  MAIN PROGRAM 												                                                                                                                                                                                    *
 ***************************************************************************************************************************************************************************************************************************************************/
 
 
int main (int argc,char *argv[ ]) {
	
	bus_struct chantilly_stack;
	uint8_t myTXBuffer[1024]={0x81,0x42,0x24,0x18}; 						// The common sequence to write to outputs. 
	uint8_t exit_loop=0;
	uint8_t c=0;
	uint32_t errors=0;
	
	sequencer seq={0xFF,0x150,1,0x50,0x04,0x141,29,0x144,2,0x145,0,0x00,0,0x1C8,0,1};	// The sructure storing specific settings of sequencer Seq.
	//seq.mask=0xFF;									
	//seq.maskAddr=0x150;								
	//seq.step=1;										
	//seq.stepAddr4=0x50;								
	//seq.cursor=0x04;								
	//seq.cursorAddr=0x141;							
	//seq.astepIncTime=29;							
	//seq.astepIncTimeAddr=0x144;	
	//seq.unfreezeResync=2;
	//seq.unfreezeResyncAddr=0x145;	
	//seq.savedCursor=0;
	//seq.savedCursorAddr=0x00;
	//seq.freezeCount=0;
	//seq.freezeCountAddr=0x1C8;	
	//seq.frozen=0;		
	//seq.freezeMask=1;			
	 
	 sequencer seq2={0xF0,0x152,1,0x71,0x04,0x1C5,29,0x1C6,0,0x1C7,0,0x02,0,0x1CA,0,4}; // The structure storing specific settings of sequencer Seq2
	//seq2.mask=0xF0;									
	//seq2.maskAddr=0x152;							
	//seq2.step=1;									
	//seq2.stepAddr4=0x71;							
	//seq2.cursor=0x04;								
	//seq2.cursorAddr=0x1C5;							
	//seq2.astepIncTime=29;							
	//seq2.astepIncTimeAddr=0x1C6;					
	//seq2.unfreezeResync=0;
	//seq2.unfreezeResyncAddr=0x1C7;
	//seq2.savedCursor=0;
	//seq2.savedCursorAddr=0x02;
	//seq2.freezeCount=0;
	//seq2.freezeCountAddr=0x1CA;		
	//seq2.frozen=0;	
	//seq2.freezeMask=4;
		
	sequencers seqs={1,0x146};										// The structure storing common settings of the enforced sequencers Seq and Seq2.	
	//seqs.wrap=1;													// SEQ_WRAP is as common setting of these sequencers.
	//seqs.wrapAddr=0x146;											//

	
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
	errors+=CHAN_setByte(IO24_NUM,seq2.mask);		// Empty writing to increase current address.
	errors+=CHAN_setByte(IO24_NUM,seq2.mask);		// Writes seq2.mask(=0xF0) at current address 0x151 into IO24. used to set seq2_MASK=0xF0
	// Since mode of sequencer is 2, Seq plays on OutputA such as OutputA=data[SEQ_CURSOR]&SEQ_MASK|OUTPUT_A with data[X]:value stored at address X and 0x04≤X≤0xFF.
	//								 Seq2 plays on OutputB such as OutputB=data[SEQ2_CURSOR]&SEQ2_MASK|OUTPUT_B.
	// With above settings Seq plays on all bits of OutputA and Seq2 plays on OutputB bit7, bit6, bit5 and bit4. 
	
	
	/** WRITES SEQUENCE VALUES */
	
	errors+=CHAN_addr(IO24_NUM,0x04);				// Sets current address at 0x04 into IO24.
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Writes sequence (stored in myTXBuffer which has been filled when initialized) at current address 0x04 into IO24.
	// Using mode 2 implies that a common sequence for Seq on OutputA and for seq2 on OutputB must be stored between included addresses 0x04 and 0xFF in RAM.
		
	
	/** SETS UP SEQUENCER Seq SETTINGS */
													// Presets via myTXBuffer :
	myTXBuffer[0]=seq.step;   						// SEQ_STEP=seq.step=1   	sequencer Seq increases.
	myTXBuffer[1]=seq.cursor;						// SEQ_CURSOR=0x04   	 	current RAM address of Seq sequence (permanently overwritten but as Seq increase must be initialized such as SEQ_CURSOR≤SEQ_END).
	myTXBuffer[2]=0x04;								// SEQ_START=0x04   	 	RAM address where (common) Seq and seq2 sequence begins.
	myTXBuffer[3]=0x07;   							// SEQ_END=0x08   		 	RAM address where (common) Seq and seq2 sequence ends. In mode 1, 0x04≤SEQ_START≤SEQ_END≤0x7F.
	myTXBuffer[4]=seq.astepIncTime;					// SEQ_ASTEP_INC_TIME    	used to change delay Ts between steps of Seq sequence such as Ts=(SEQ_ASTEP_INC_TIME+1)*Tseq=31*0.005014=155.434 ms. => fs=6.43Hz. 
	myTXBuffer[5]=seq.unfreezeResync;				// SEQ_UNFREEZE_RESYNC=2 	Seq is synchronized with Seq2 when released : Seq2 is time reference for Seq.
	myTXBuffer[6]=seqs.wrap;						// SEQ_WRAP=1   		 	plays sequences infinitely by setting SEQ_CURSOR=SEQ_START if SEQ_STEP=1 or SEQ_CURSOR=SEQ_END if SEQ_STEP=2 when Seq sequence ends.
	myTXBuffer[7]=2;								// SEQ_MODE=2   		 	the two sequencers Seq and Seq2 are independent and play a common sequence asynchronously. That means sequencers Seq and Seq2 settings are required.
	errors+=CHAN_addr4(IO24_NUM,0x50);				// Sets current address at 0x50*4=0x140 into IO24.
	errors+=CHAN_setBytes(IO24_NUM,8,myTXBuffer);	// Writes sequencer Seq settings (stored in myTXBuffer) at current address.
	// To edit SEQ_MODE and SEQ_WRAP, sequencer mode must be restarted by executing IO24 command 0x03 named SEQUENCER_RUN. (SEQ_MODE and SEQ_WRAP are common settings to the three sequencers).
	// All others sequencer settings can be edited when sequenced outputs mode is running but it is not advisable.
	
	
	/** SETS UP SEQUENCER seq2 SETTINGS */
													// Presets via myTXBuffer :
	myTXBuffer[0]=seq2.step;   						// SEQ2_STEP=seq2.step=1   	sequencer seq2 increases.
	myTXBuffer[1]=seq2.cursor;						// SEQ2_CURSOR=0x04   	   	current RAM address of seq2 sequence (permanently overwritten but as seq2 increase must be initialized such as SEQ2_CURSOR≤SEQ_END).
	myTXBuffer[2]=seq2.astepIncTime;				// SEQ2_ASTEP_INC_TIME     	used to change delay Ts2 between steps of seq2 sequence such as Ts2=(SEQ2_ASTEP_INC_TIME+1)*Tseq=31*5.014ms=155.434 ms. => fs2=6.43Hz. 
	myTXBuffer[3]=seq2.unfreezeResync;				// SEQ2_UNFREEZE_RESYNC=0   Seq1 is synchronized with Seq when released : Seq is time reference for Seq1.
	errors+=CHAN_addr4(IO24_NUM,0x71);				// sets current address at 0x70*4=0x1C0 into IO24.
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// writes sequencer seq2 settings (stored in myTXBuffer) at current address 0x1C0 in RAM.
	// All sequencer Seq2 settings can be edited when sequenced outputs mode is running but it is not advisable.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											STARTS SEQUENCED OUTPUTS MODE																																										*
	************************************************************************************************************************************************************************************************************************************************/											
	
	
	errors+=CHAN_command(IO24_NUM,0x03);			// Executes IO24 command 0x03 named SEQUENCER_RUN : Starts (immediate inputs and) sequenced outputs mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											LOOPS :			             	                                                                                                                                                                    *
	************************************************************************************************************************************************************************************************************************************************/


	printf("\nIO24 sequenced outputs mode 2 is started : \n");
	printf("Sequencers Seq and Seq2 are running with respective steps delays Ts and Ts2 set such as ");
	printf("Ts=Tseq*(SEQ_ASTEP_INC_TIME+1) and Ts2=Tseq*(SEQ2_ASTEP_INC_TIME+1) ");
	printf("with SEQ_ASTEP_INC_TIME=SEQ2_ASTEP_INC_TIME=29 and Tseq=5ms ");
	printf("then Ts=Ts2=30*5ms=150ms\n\n");
	displayMainMenu();
	
	while (!exit_loop) {												// Entering loop
		getch_start();													//
		c=getch_try();													//
		getch_stop();													//  Gets the last character typed without stopping program (see FUNCTIONS above).
		if (c) {                                                    	//  if something is typed :
			if (c=='q') exit_loop=1;									//   if 'q' is typed exits loop.
			else if (c=='w') errors+=invertWrap(&seqs);					//	 if 'w' is typed inverts wrap of the three sequencers. invertWrap() takes &seqs as parameter because SEQ_WRAP is a common parameter to the three sequencers. 
			else if (c=='p') errors+=pauseResumeSteps(&seq,&seq2);		//	 if 'p' is typed pauses/resumes the three sequencers. pauseSteps() takes &seq and &seq2 as parameters because SEQ_STEP and SEQ2_STEP are independent.
			else {														
				if (c=='0') errors+=displaySeqMenu(c,&seq,&exit_loop); 	// 	  if '0' is typed shows Seq menu.
				if (c=='2') errors+=displaySeqMenu(c,&seq2,&exit_loop);	//    if '2' is typed shows Seq2 menu.
			}														
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
