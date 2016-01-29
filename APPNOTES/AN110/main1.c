 /*******************************************************************
 *                                                                  * 
 * Chantilly 0x10 - IO24 : Source code for application note AN110   *
 *                                                     			    *
 ****************************************************************************************************************************************************************************************************************************************************
 * CIRCUIT DESCRIPTION :																																					                                                                        *
 * Core module IO24 is set at number 1 on chantilly stack (redefine IO24_NUM according to hardware-address to change it : between 1 included and 6 included).																						*
 * As described in file chantilly_0x10_AN110_pcb2rev1.PDF, a bar led is wired on OutputA and OutputB.																																				*
 * 																																																													*
 *  																																																												* 
 * PROGRAM DESCRIPTION : Test using IO24 direct core access mode with (immediate inputs and) sequenced outputs in mode 0.                                                                    		                                               	*
 * A couple of synchronous sequences is written in free RAM.																																														* 
 * Due to mode 0, sequencers Seq play the first sequence of the couple on OutputA and the second sequence of the couple on OutputB.  																												*
 * SEQ_MASK, SEQ1_MASk initial values make Seq outputted on all bits of OutputA and on all bits of OutputB.																																			*
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
{																		// Even if in mode 0 only sequencer Seq is enforced, SEQ_WRAP is treated as common setting to the three sequencers.  
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
	if (seqN->step==1) seqN->cursor=0x04;								// Using mode 0 implies Seq or Seq1 can be passed in parameter.
	else seqN->cursor=0x07;
	*errors+=CHAN_addr(IO24_NUM,seqN->cursorAddr);
	*errors+=CHAN_setByte(IO24_NUM,seqN->cursor);
}

void displayMainMenu() {
	printf("\nMain menu\n");
	printf("Type :\t'0' to display Sequencer Seq menu.\n");
	printf("\t'1' to display Sequencer Seq1 menu.\n");
	printf("\t'w' to invert wrap.\n");
	printf("\t'p' to pause/resume.\n");
	printf("\t'q' to exit.\n");
}

uint32_t displaySeqMenu(char c,sequencer* seqN,uint8_t* exit_loop) { 					// displaySeqMenu() lets the user edit specific settings of the sequencer passed in parameter.
	uint8_t c2='\0';																	// Using mode 0 implies Seq or Seq1 can be passed in parameter.
	uint32_t errors=0;																	// Seq1 is not enforced in this mode but SEQ1_MASK is used by Seq. 
	static uint8_t frozen=0;															// So passing Seq1 as parameter to the function is used to edit SEQ1_MASK only.
	if (c!='0') {
		printf("\nSequencer Seq%c menu\n",c);
		printf("\tType\t:\t'm' to edit mask.\n");
		printf("\t\t\t'r' to return.\n");
		printf("\t\t\t'q' to quit.\n");
	}
	else {
		printf("\nSequencer Seq menu\n");
		printf("\tType\t:\t'm' to edit mask.\n");
		printf("\t\t\t's' to invert step.\n");
		printf("\t\t\t'd' to change steps delay.\n");
		printf("\t\t\t'f' to freeze/release.\n");
		printf("\t\t\t'g' to freeze & force/release.\n");
		printf("\t\t\t'r' to return.\n");
		printf("\t\t\t'q' to quit.\n");
	}
	fflush(stdout);
	while (!((c2=='q'||c2=='m'||c2=='d'||c2=='r'||c2=='s'||c2=='f') && c=='0' ) 		// if Seq is passed in parameter to the function, characters typed can be 'm', 'd', 'r', 's', 'f',
						&&  !((c2=='q'||c2=='m'||c2=='r') && c=='1' )) {				// if Seq1 is passed in parameter to the function, characters typed can be 'm', 'r' or 'q'.
		getch_start();																	//
		c2=getch_try();																	//
		getch_stop();																	// Gets the last character typed without stopping program (see FUNCTIONS above).
	}
	if (c2) {
		fflush (stdout);
		if (c2=='q') *exit_loop=1;														// If character typed is 'q' : Exits loop.
		else {
			if (c2=='m') {																// If character typed is 'm' : Edits the mask.
				if (c!='0') {
					printf("\nSEQ%c_MASK = 0x%02X\t",c,seqN->mask);
					printf("Enter new value for SEQ%c_MASK = 0x",c);
				}
				else {
					printf("\nSEQ_MASK = 0x%02X\t",seqN->mask);
					printf("Enter new value for SEQ_MASK = 0x");
				}			
				scanf("%2x",(unsigned int*)&seqN->mask);
				errors+=CHAN_addr(IO24_NUM,seqN->maskAddr);								// Switch seqN parameter passed to the function, sets current address at address of SEQ_MASK or SEQ1_MASK into IO24.
				errors+=CHAN_setByte(IO24_NUM,seqN->mask);								// Writes seqN.mask at current address.		
			}
			else if (c2=='s') {															// If character typed is 's' : Edits the sense.
				if (seqN->step==1) seqN->step=2;
				else if (seqN->step==2) seqN->step=0;
				else seqN->step=1;
				errors+=CHAN_addr4(IO24_NUM,seqN->stepAddr4);							//  Sets current address at address of SEQ_STEP into IO24.
				errors+=CHAN_setByte(IO24_NUM,seqN->step);								//  Writes seqN.step at current address.
				if (seqN->step==0) printf("sequencer Seq does nothing\n");				//
				else if (seqN->step==1) printf("sequencer Seq increases\n");			//
				else if (seqN->step==2) printf("sequencer Seq decreases\n");			//  Displays the new sense of the sequencer Seq. 			
			}
			else if (c2=='d') {															// If character typed is 'd' : Edits delay between sequence steps. 
				printf("Seq steps delay Ts is such as Ts=Tseq*(SEQ_ASTEP_INC_TIME+1)");	//  Displays SEQ_ASTEP_INC_TIME and current steps delay of the sequencer Seq. 
				printf(" with Tseq=5ms and SEQ_ASTEP_INC_TIME=%d\n",seqN->astepIncTime);//   
				printf("Enter new value for SEQ_ASTEP_INC_TIME=");						//   
																						//  Asks a new value for SEQ_ASTEP_INC_TIME.
				scanf("%3d",(unsigned int*)&seqN->astepIncTime);						//  Stores the new value of SEQ_ASTEP_INC_TIME into variable seqN.astepIncTime.
					
				errors+=CHAN_addr(IO24_NUM,seqN->astepIncTimeAddr);						//  Sets current address at address of SEQ_ASTEP_INC_TIME.
				errors+=CHAN_setByte(IO24_NUM,seqN->astepIncTime);						//  Writes seqN.astepIncTime at current address.
				printf("Seq steps delay Ts is now set such as Ts=Tseq*");				//
				printf("(SEQ_ASTEP_INC_TIME+1)=%dms\n",(seqN->astepIncTime+1)*5);		//   Displays new steps delay of Seq.				
			}																			//  
			else if (c2=='f') {															// If character typed is 'f' : Freezes/releases sequencer.
				errors+=CHAN_addr(IO24_NUM,0x1CB);										//  Sets current address at address of SEQS_FREEZE_MASK.														
				if (!frozen) {															//   If Seq is not frozen.
					errors+=CHAN_setByte(IO24_NUM,1);									//    Writes 1 at current address to define Seq as sequencer to freeze.
					CHAN_command(IO24_NUM,2);											//    Executes IO24 command 0x02 named FREEZE : The sequencers defined by SEQS_FREEZE_MASK are frozen.
					frozen=1;
					printf("Sequencer Seq is frozen.\n");
					fflush(stdout);
				}
				else {																	//   Else
					errors+=CHAN_addr(IO24_NUM,0x14B);									//    Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
					errors+=CHAN_setByte(IO24_NUM,0);									//    Writes 0 at current address : To synchronize Seq with any step of Seq.	 
					CHAN_command(IO24_NUM,4);											//    Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
					frozen=0;
					printf("Sequencer Seq is released.\n");
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
					fflush(stdout);
				}
			}
			else if (c2=='f') {															// If character typed is 'f' : Freezes/releases sequencer.
				errors+=CHAN_addr(IO24_NUM,0x1CB);										//  Sets current address at address of SEQS_FREEZE_MASK.														
				if (!frozen) {															//   If Seq is not frozen.
					errors+=CHAN_setByte(IO24_NUM,1);									//    Writes 1 at current address to define Seq as sequencer to freeze.
					CHAN_command(IO24_NUM,2);											//    Executes IO24 command 0x02 named FREEZE : The sequencers defined by SEQS_FREEZE_MASK are frozen.
					frozen=1;
					printf("Sequencer Seq is frozen.\n");
					fflush(stdout);
				}
				else {																	//   Else
					errors+=CHAN_addr(IO24_NUM,0x14B);									//    Sets current address at address of SEQS_UNFREEZE_POSITION into IO24.
					errors+=CHAN_setByte(IO24_NUM,0);									//    Writes 0 at current address : To synchronize Seq with any step of Seq.	 
					CHAN_command(IO24_NUM,4);											//    Executes IO24 command 0x04 named RELEASE : The sequencers defined by SEQS_FREEZE_MASK are released and synchronized.
					frozen=0;
					printf("Sequencer Seq is released.\n");
					fflush(stdout);
				}
			}
			if (c2!='r') return displaySeqMenu(c,seqN,exit_loop);						// If charactere typed is not 'r' displays recursively menu of the sequencer passed to the function.
			else printf("return\n");													// Else 
			displayMainMenu();															//  Displays main menu.
		}
	}
	return errors;
}

uint32_t invertWrap(sequencers* seqs) {								 	// invertWrap() sets SEQ_WRAP to 0 or 1 switch previous value. SEQ_WRAP is common setting to the three sequencers.
	uint32_t errors=0;													// So seqs is passed in parameter to the function.
	if (seqs->wrap==0) {												//	
		seqs->wrap=1;													//															
	}																	//
	else seqs->wrap=0;													// Sets variable seqs.wrap to 0 or 1 switch its previous value.
	errors+=CHAN_addr(IO24_NUM,seqs->wrapAddr);							// Sets current address at address of SEQ_WRAP.
	errors+=CHAN_setByte(IO24_NUM,seqs->wrap);							// Writes value of seqs.wrap at current address.
	printf("wrap set to %d\n",seqs->wrap);
	displayMainMenu();
	return errors;
}

uint32_t pauseResumeStep(sequencer* seqN) { 				  			// pauseResumeStep() pauses/resumes sequencer passed in parameter.
	uint32_t errors=0;													// Using mode 0 implies only Seq should be passed in parameter.
	static uint8_t stepN=0;												//
	if (!stepN) {														// If Seq increases or decreases.
		printf("paused\n");												//
		stepN=seqN->step;												// 	Saves value of seqN.Step in stepN.
		seqN->step=0;													//  Sets seqN.step to 0.
	}
		else {															// Else
		printf("running\n");											//  
		seqN->step=stepN;												//	Sets seqN.step to saved value of stepN.
		stepN=0;														//  Resets stepN.
	}
	errors+=CHAN_addr4(IO24_NUM,seqN->stepAddr4);						// Sets current address at address of SEQ_STEP.
	errors+=CHAN_setByte(IO24_NUM,seqN->step);							// Writes value of seqN.step at current address.
	displayMainMenu();													//  Displays main menu.
	return errors;
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
		
	sequencer seq={0xFF,0x150,1,0x50,0x04,0x141,30,0x144,0,0x145};	// The sructure storing specific settings of sequencer Seq.
	//seq.mask=0xFF;								 
	//seq.maskAddr=0x150;							 
	//seq.step=1;									  									
	//seq.stepAddr4=0x50;														
	//seq.cursor=0x04;								
	//seq.cursorAddr=0x141;							
	//seq.astepIncTime=30;							
	//seq.astepIncTimeAddr=0x144;
	//seq.unfreezeResync=0;
	//seq.unfreezeResyncAddr=0x145;						
	
	sequencer seq1={0xFF,0x151};									// The sructure storing specific settings of sequencer Seq1.
	//seq1.mask=0xFF;												// only usefull members of seq1 are initialized : seq1.mask and seq1.maskAddr.
	//seq1.maskAddr=0x151;											// SEQ1_MASK is used by Seq in mode 0.
	
	sequencers seqs={1,0x146};										// The structure storing common settings of the three sequencers Seq Seq1 and Seq2 even if in mode 0 only Seq is enforced.
	//seqs.wrap=1;													// SEQ_WRAP is treated as common setting of the three sequencers.
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
	errors+=CHAN_setByte(IO24_NUM,seq1.mask);		// Writes seq1.mask(=0xFF) at current address 0x151 into IO24. used to set SEQ1_MASK=0xFF
	// Since mode of sequencer is 0, Seq play on OutputA such as OutputA=data[SEQ_CURSOR]&SEQ_MASK|OUTPUT_A with data[X]:value stored at address X and 0x04≤X≤0xFF.
	//								 Seq plays on OutputB such as OutputB=data[SEQ_CURSOR+128]&SEQ1_MASK|OUTPUT_B.
	// With above settings Seq plays on all bits of OutputA and on all bits of OutputB. 
	
	
	/** WRITES SEQUENCES VALUES */
	
	myTXBuffer[00]=0x81;							// Seq sequence on OutputA			
	myTXBuffer[01]=0x42;							// (1st sequence of the couple)	
	myTXBuffer[02]=0x24;							//  							
	myTXBuffer[03]=0x18;							//						
	errors+=CHAN_addr(IO24_NUM,0x04);				// Sets current address at 0x04 into IO24.
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Writes sequence (via myTXBuffer) at current address 0x04 into IO24.
	// Using mode 0 implies that first sequence of the couple is the sequence of Seq on OutputA. This sequence must be stored between included addresses 0x04 and 0x7F in RAM.
	
	myTXBuffer[00]=0x80;							// Seq sequence on OutputB			
	myTXBuffer[01]=0x40;							// (2nd sequence of the couple)	
	myTXBuffer[02]=0x20;							//  							
	myTXBuffer[03]=0x10;							//								
	errors+=CHAN_addr(IO24_NUM,0x84);				// Sets current address at 0x84 into IO24.
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Writes sequence (via myTXBuffer) at current address 0x04 into IO24.
	// Using mode 0 implies that second sequence of the couple is the sequence of Seq on OutputB. This sequence must be stored between included addresses 0x84 and 0xFF in RAM.
	
	
	/** SETS UP SEQUENCER Seq SETTINGS */
													// Presets via myTXBuffer :
	myTXBuffer[0]=seq.step;   						// SEQ_STEP=seq.step=1 : sequencer Seq increases.
	myTXBuffer[1]=seq.cursor;						// SEQ_CURSOR=0x04 : 	 current RAM address of Seq sequence (permanently overwritten but as Seq increase must be initialized such as SEQ_CURSOR≤SEQ_END).
	myTXBuffer[2]=0x04;								// SEQ_START=0x04 : 	 RAM address where (common) Seq sequence begins.
	myTXBuffer[3]=0x07;   							// SEQ_END=0x08 : 		 RAM address where (common) Seq sequence ends. In mode 0, 0x04≤SEQ_START≤SEQ_END≤0x7F.
	myTXBuffer[4]=seq.astepIncTime;					// SEQ_ASTEP_INC_TIME :  used to change delay Ts between steps of Seq sequence such as Ts=(SEQ_ASTEP_INC_TIME+1)*Tseq=31*0.005014=155.434 ms. => fs=6.43Hz. 
	myTXBuffer[5]=seq.unfreezeResync;				// SEQ_UNFREEZE_RESYNC=0 Seq is synchronized with itself when released.
	myTXBuffer[6]=seqs.wrap;						// SEQ_WRAP=1 : 		 plays sequences infinitely by setting SEQ_CURSOR=SEQ_START if SEQ_STEP=1 or SEQ_CURSOR=SEQ_END if SEQ_STEP=2 when Seq sequence ends.
	myTXBuffer[7]=0;								// SEQ_MODE=0 : 		 mode 0, only Seq settings are required.
	errors+=CHAN_addr4(IO24_NUM,0x50);				// Sets current address at 0x50*4=0x140 into IO24.
	errors+=CHAN_setBytes(IO24_NUM,8,myTXBuffer);	// Writes sequencer Seq settings (stored in myTXBuffer) at current address.
	// To edit SEQ_MODE and SEQ_WRAP, sequencer mode must be restarted by executing IO24 command 0x03 named SEQUENCER_RUN. 
	// All others sequencer settings can be edited when stepcount sequenced outputs mode is running but it is not advisable.
		
	
	/************************************************************************************************************************************************************************************************************************************************
	*											STARTS SEQUENCED OUTPUTS MODE																																										*
	************************************************************************************************************************************************************************************************************************************************/											
	
	
	errors+=CHAN_command(IO24_NUM,0x03);			// Executes IO24 command 0x03 named SEQUENCER_RUN : Starts (immediate inputs and) sequenced outputs mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											LOOPS :			             	                                                                                                                                                                    *
	************************************************************************************************************************************************************************************************************************************************/


	printf("\nIO24 sequenced outputs mode 0 is started : \n");
	printf("Sequencers Seq is running with steps delay Ts set such as ");
	printf("Ts=Tseq*(SEQ_ASTEP_INC_TIME+1)");
	printf("with SEQ_ASTEP_INC_TIME=30 and Tseq=5ms ");
	printf("then Ts=31*5ms=155ms\n\n");
	displayMainMenu();
	CHAN_command(IO24_NUM,4);
	while (!exit_loop) {															// Entering loop
		getch_start();																//
		c=getch_try();																//
		getch_stop();																//  Gets the last character typed without stopping program (see FUNCTIONS above).
		if (c) {                                                    				//  If something is typed :
			if (c=='q') exit_loop=1;												//   If 'q' is typed exits loop.
			else if (c=='w') errors+=invertWrap(&seqs);								//	 If 'w' is typed inverts wrap of the three sequencers. invertWrap() takes &seqs as parameter because SEQ_WRAP is a common parameter to the three sequencers. 
			else if (c=='p') errors+=pauseResumeStep(&seq);									//	 If 'p' is typed pauses/resumes the three sequencers. pauseSteps() takes &seq as parameters because SEQ_STEP is specific to sequencer Seq.
			else {														
				if (c=='0') errors+=displaySeqMenu(c,&seq,&exit_loop); 				// 	  If '0' is typed shows Seq menu.
				if (c=='1') errors+=displaySeqMenu(c,&seq1,&exit_loop);				//    If '1' is typed shows Seq1 menu.
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
