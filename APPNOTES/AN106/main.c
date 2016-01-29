 /*******************************************************************
 *                                                                  *  
 * Chantilly 0x10 - IO24 : Source code for application note AN106   *
 *                                                     			    *
 ****************************************************************************************************************************************************************************************************************************************************
 * CIRCUIT DESCRIPTION :                                                                                                                                                                           													*
 * Core module IO24 is set at number 1 on chantilly stack (redefine IO24_NUM according to hardware-address to change it : between 1 included and 6 included).	                                   													*
 * As described in file chantilly_0x10_AN106_pcb2rev1.PDF, an 8columns*5lines led matrice is wired on OutputA for columns and OutputB for lines. Switch1 is wired on InputA bit0.																	*
 *  			 																																																									*
 * 																																																													*
 * PROGRAM DESCRIPTION : Test using IO24 direct core access mode with (immediate inputs and) sequenced outputs mode 0.																																*
 * 3 Couples of synchronous sequences are written in free RAM. The first sequence of each couples of sequences is an columns sequence. The second one is an lines sequence.																			*
 * Sequencer Seq plays on OutputA columns sequences and sequencer Seq1 plays on OutputB lines sequences.                                                                                                                                            *
 * By pressing on switch1, the user can change the current couple of sequences.  												                               					       																*
 * By typing on keyboard, lets the user change the sequence update frequency or exit program. 	                                                                                                                                                   	*
 *																																																													*
 * 																																																													*
 * Commented lines mainly refers to chantilly use.												                              					          										 													*
 ***************************************************************************************************************************************************************************************************************************************************/
  
#define IO24_NUM 1 									// IO24 is core number 1 on chantilly stack. (Choosen by user but must match hardware-address on IO24 Board)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/ioctl.h>

#include <CHANTILLY_BP.h>


 /***************************************************************************************************************************************************************************************************************************************************
 *  FUNCTIONS 																																														                                                *
 ***************************************************************************************************************************************************************************************************************************************************/
 
 
int kbhit(void)
{
	 int i;
	 ioctl(0, FIONREAD, &i);
	 return i; 										// Returns a count of characters available to read.
}

void getch_start(void)	{  system("stty raw -echo"); }

void getch_stop(void) { system("stty cooked echo"); }

unsigned char getch_try(void)               		// Gets the last character typed without stopping program.
{
	int c=0;
	while (kbhit()) { c=getchar(); }
	return (unsigned char) c;						// One char from queue only.
}	


 /***************************************************************************************************************************************************************************************************************************************************
 *  MAIN PROGRAM                                                                                                                                                                                                                                    *
 ***************************************************************************************************************************************************************************************************************************************************/
 
 
int main (int argc,char *argv[ ]) {
	
	bus_struct chantilly_stack;
	uint8_t myTXBuffer[1024]={0};
	uint8_t myRXBuffer[1024];
	uint8_t exit_loop=0;
	uint32_t errors=0;
	uint8_t myLastInput=0;
	uint8_t i=0;
	uint8_t c=0;
	float presca=1.0/2.0;			
	float f_internal=10000000.0; 			// Internal frequency 10MHz.
	float f_requested=0.0;
	uint8_t inc_time=0;
	uint16_t FFFFseqUserVal=0.0;			// Variable storing value of 0xFFFF-seqUserVal.
	float diff=100.0;
	uint16_t seqUserVal=0.0;		
	uint8_t seqUserVal_l=0;
	uint8_t seqUserVal_h=0;
	uint8_t seq_cursor=0;
	uint8_t seq_start=0;
	uint8_t seq_end=0;
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP CHANTILLY 				                                                                                                                                                                    *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	if(CHAN_setup("APP100",1)!=0) { printf("Error opening chantilly , error or processID { }\n"); CHAN_close();return 0;}	// Stops here if can't open immediate core access mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											CHECKS CHANTILLY STACK 				                                                                                                                                                                *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	CHAN_checkBUS(&chantilly_stack,1,1);			// Checks what's on the addr1 of the chantilly stack bus
	CHAN_checkBUS(&chantilly_stack,2,1);			// Checks what's on the addr2 of the chantilly stack bus
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP UPDATE PERIOD OF SEQUENCED OUTPUTS MODE 				                                                                                                                                    *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	errors+=CHAN_addr(IO24_NUM,0x148);          	// Sets current address to 0x148 into IO24 
	errors+=CHAN_setByte(IO24_NUM,0x08);       		// Writes 0x08 at current address 0x148 into IO24 (current address increases automatically).
	errors+=CHAN_setByte(IO24_NUM,0xCF);			// Writes 0xCF at current address 0x149 into IO24 (current address increases automatically).  
													// Two previous values at addresses 0x148 and 0x149 are used to set SEQ_USER_VAL=0xCF08 => 0xFFFF-SEQ_USER_VAL=0x30F7=12535.
	errors+=CHAN_setByte(IO24_NUM,0);				// Writes 0x00 at current address 0x14A into IO24.
													// Previous value is used to set SEQ_USER_PRESCA=0 => PRESCAseq=1/2 (0,1,2,..,7 respectively corresponding to 1/2,1/4,1/8,..,1/256). 
	// Sequenced outputs mode is now set with update period, Tseq, such as Tseq=(0xFFFF-SEQ_USER_VAL)*100ns/PRESCAseq=12535*100ns*2=2.507ms.					 
	// That means inputs are read every 2.507ms and sequenced outputs are updated every (SEQ_ASTEP_INC_TIME+1)*Tseq=(SEQ_ASTEP_INC_TIME+1)*2.507ms.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP SEQUENCED OUTPUTS MODE				                                                                                                                                                        *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	/** SETS UP SEQUENCED OUTPUTS */		
		
	errors+=CHAN_addr4(IO24_NUM,0x40);				// Sets current address at 0x40*4=0x100 into IO24.
	errors+=CHAN_setByte(IO24_NUM,0);				// Writes 0 at current address 0x100 into IO24. Used to set OUTPUT_A=0. 
	errors+=CHAN_setByte(IO24_NUM,0);				// Writes 0 at current address 0x101 into IO24. Used to set OUTPUT_B=0. 			
	errors+=CHAN_addr4(IO24_NUM,0x54);				// Sets current address at 0x54*4=0x150 into IO24.
	errors+=CHAN_setByte(IO24_NUM,0xFF);			// Writes 0xFF at current address 0x150 into IO24. Used to set SEQ_MASK=0xFF.
	errors+=CHAN_setByte(IO24_NUM,0xF8);			// Writes 0xF8 at current address 0x151 into IO24. Used to set SEQ1_MASK=0xF8.
	// Since mode of sequencer is 0, Seq plays on OutputA such as OutputA=(data[SEQ_CURSOR]&SEQ_MASK)|OUTPUT_A) with data[X]:value stored at address X and 0x04≤X≤0x7F.
	//								 Seq1 plays on OutputB such as OutputB=(data[SEQ_CURSOR+128]&SEQ1_MASK)|OUTPUT_B).
	// With previous settings, Seq plays on all bits of OutputA and Seq1 plays on OutputB bit7, bit6, bit5, bit4 and bit3.
	
	
	/** WRITES Seq SEQUENCES VALUES */
	
	myTXBuffer[00]=0b00000000;						// 1st Seq sequence 			// columns sequences
	myTXBuffer[01]=0b11111111;						// (1st sequence of 1st couple)	//
	myTXBuffer[02]=0b11111111;						//  							//
	myTXBuffer[03]=0b11111111;						//								//
	myTXBuffer[04]=0b11111111;						//								//
																					//
	myTXBuffer[05]=0b10000001;						// 2nd Seq sequence				//
	myTXBuffer[06]=0b01111110;						// (1st sequence of 2nd couple)	//
	myTXBuffer[07]=0b01111110;						// 								//
	myTXBuffer[8]=0b01111110;						// 								//
	myTXBuffer[9]=0b10000001;						//								//
																					//
	myTXBuffer[10]=0b01111111;						// 3rd Seq sequence				//
	myTXBuffer[11]=0b11111110;						// (1st sequence of 3rd couple)	//
	myTXBuffer[12]=0b10111111;						//								//
	myTXBuffer[13]=0b11111101;						//								//
	myTXBuffer[14]=0b11011111;						//								//
	myTXBuffer[15]=0b11111011;						//								//
	myTXBuffer[16]=0b11101111;						//								//
	myTXBuffer[17]=0b11110111;						// 								//
	myTXBuffer[18]=0b11110111;						//								//
	myTXBuffer[19]=0b11101111;						//								//
	myTXBuffer[20]=0b11111011;						//								//
	myTXBuffer[21]=0b11011111;						//								//
	myTXBuffer[22]=0b11111101;						//								//
	myTXBuffer[23]=0b10111111;						//								//
	myTXBuffer[24]=0b11111110;						//								//
	myTXBuffer[25]=0b01111111;						// 								//
		
	errors+=CHAN_addr(IO24_NUM,0x04);				// Sets current address at 0x04 into core IO24.											
	errors+=CHAN_setBytes(IO24_NUM,26,myTXBuffer);	// Writes Seq sequences (stored in myTXBuffer) at current address 0x04 into core IO24 (myTXBuffer has been filled when initialized).
	// Using mode 0 (for dual sequencer enforcing Seq and Seq1) implies that Seq sequences must be stored between included addresses 0x04 and 0x7F in RAM.
	
	
	/** WRITES Seq1 SEQUENCES VALUES */
	
	myTXBuffer[00]=0b10000000;						// 1st Seq1 sequence			// lines sequences
	myTXBuffer[01]=0b01000000;						// (2nd sequence of 1st couple)	//
	myTXBuffer[02]=0b00100000;						//								//
	myTXBuffer[03]=0b00010000;						//								//
	myTXBuffer[04]=0b00001000;						//								//
																					//
	myTXBuffer[05]=0b10000000;						// 2nd Seq1 sequence			//
	myTXBuffer[06]=0b01000000;						// (2nd sequence of 2nd couple)	//
	myTXBuffer[07]=0b00100000;						//								//
	myTXBuffer[8]=0b00010000;						//								//
	myTXBuffer[9]=0b00001000;						//								//
																					//
	myTXBuffer[10]=0b10101000;						// 3rd Seq1 sequence			//
	myTXBuffer[11]=0b01010000;						// (2nd sequence of 3rd couple)	//
	myTXBuffer[12]=0b10100000;						//								//
	myTXBuffer[13]=0b01010000;						//								//
	myTXBuffer[14]=0b10101000;						//								//
	myTXBuffer[15]=0b01010000;						//								//
	myTXBuffer[16]=0b10101000;						//								//
	myTXBuffer[17]=0b01010000;						//								//
	myTXBuffer[18]=0b10101000;						//								//
	myTXBuffer[19]=0b01010000;						//								//
	myTXBuffer[20]=0b10100000;						//								//
	myTXBuffer[21]=0b01010000;						//								//
	myTXBuffer[22]=0b10101000;						//								//
	myTXBuffer[23]=0b01010000;						//								//
	myTXBuffer[24]=0b10101000;						//								//
	myTXBuffer[25]=0b01010000;						// 								//
	
	errors+=CHAN_addr(IO24_NUM,128+4);				// Sets current address at 0x84 into core IO24.						
	errors+=CHAN_setBytes(IO24_NUM,26,myTXBuffer);	// Writes Seq and Seq1 sequences (stored in myTXBuffer) at current address 0x84 into core IO24.
	// Using mode 0 (for dual sequencer enforcing Seq and Seq1) implies that Seq1 sequences must be stored between included addresses 0x84 and 0xFF in RAM. 
	
	
	/** SETS UP SEQUENCER Seq SETTINGS */			// Using mode 0 so Seq and Seq1 sequences are synchronous. Only Seq sequence settings are required, Seq1 sequence settings depend on Seq sequence setting. 
													
													// Presets via myTXBuffer :
	myTXBuffer[0]=1;   								// SEQ_STEP=1 : 		sequencer Seq increases. 	
	myTXBuffer[1]=0x04;								// SEQ_CURSOR=0x04 : 	current RAM address of (first) Seq sequence (permanently overwritten but as Seq increase must be initialized such as SEQ_CURSOR≤SEQ_END).
	myTXBuffer[2]=0x04;								// SEQ_START=0x04 : 	RAM address where (first) Seq sequence begins.  
	myTXBuffer[3]=0x08;   							// SEQ_END=0x08 : 		RAM address where (first) Seq sequence ends. In mode 0, 0x04≤SEQ_START≤SEQ_END≤0x7F.
	myTXBuffer[4]=0x00;								// SEQ_ASTEP_INC_TIME : used to change delay Ts between steps of (first) Seq sequence such as Ts=(SEQ_ASTEP_INC_TIME+1)*Tseq=1*0.002507=2.507ms => fs=398.883127244Hz.
	myTXBuffer[5]=0;								/*********************************************non implementé ??????************************/
	myTXBuffer[6]=1;								// SEQ_WRAP=1 : 		plays sequence infinitely by setting SEQ_CURSOR=SEQ_START when (first) Seq sequence ends.
	myTXBuffer[7]=0;								// SEQ_MODE=0 : 		mode 0 to use dual synchronous sequencer enforcing Seq and Seq1. That means Seq1 sequence settings are not required. 
	// All eight previous parameters can be edited without having to stop running mode. They take effects when executing IO24 command 0x03 named SEQUENCER_RUN.
	errors+=CHAN_addr4(IO24_NUM,0x50);				// Sets current address at 0x50*4=0x140 into IO24.
	errors+=CHAN_setBytes(IO24_NUM,8,myTXBuffer);	// Writes current Seq sequence settings (stored in myTXBuffer) in RAM at current address 0x140.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											STARTS SEQUENCED OUTPUTS MODE																																										*
	************************************************************************************************************************************************************************************************************************************************/											
	
	
	errors+=CHAN_command(IO24_NUM,0x03);			// Executes IO24 command 0x03 named SEQUENCER_RUN : Starts IO24 (immediate inputs and) sequenced outputs mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											PLAYS SEQUENCE		     		                                                                                                  																    *
	************************************************************************************************************************************************************************************************************************************************/
	
	printf("\nIO24 sequenced outputs mode 0 is started : \nSequencers Seq and Seq1 are running with \tsteps delay Ts set such as Ts=2.507ms => fs=1/Ts=398.8831Hz\n\n");
	printf("Press switch1 to change sequence, type 'e' to edit frequency or 'q' to exit.\n");												
	
	while (!exit_loop) {
		errors+=CHAN_addr4(IO24_NUM,0x40);
		errors+=CHAN_getBytes(IO24_NUM,3,myRXBuffer);
		if ((myRXBuffer[2]&0x01)&&!(myLastInput&0x01)){							//  if switch1 is pressed
			myLastInput=myLastInput|0b00000001;									//   Acknowledges : prevents repetition while reading inputs.
			if (i==0) {															//   if switch1 is pressed for the first time :
				seq_cursor=0x09;												//  
				seq_start=0x09;													//    presets 2nd sequence
				seq_end=0x0d;													//
				i++;															//    increases the number of times switch1 has been pressed.
				printf("Second sequence set. \n\nPress switch1 to change sequence, type 'e' to edit frequency or 'q' to exit.\n");
			}
			else if (i==1) {													//   else if switch1 is pressed for the second time :
				seq_cursor=0x0e;												//	
				seq_start=0x0e;													//    presets 3rd sequence
				seq_end=0x1d;													//    increases the number of times switch1 has been pressed.
				i++;																						
				printf("Third sequence set. \n\nPress switch1 to change sequence, type 'e' to edit frequency or 'q' to exit.\n");
			}
			else if (i==2) {													//   else if switch1 is pressed for the third time :
				seq_cursor=0x04;												//
				seq_start=0x04;													//    presets 1st sequence
				seq_end=0x08;													//	 
				i=0;															//	  resets the number of times switch1 has been pressed.
				printf("First sequence set. \n\nPress switch1 to change sequence, type 'e' to edit frequency or 'q' to exit.\n");
			}
			errors+=CHAN_addr(IO24_NUM,0x141);									//   Sets current address at 0x141 into IO24.
			errors+=CHAN_setByte(IO24_NUM,seq_cursor);							//   Sets SEQ_CURSOR=seq_cursor.
			errors+=CHAN_setByte(IO24_NUM,seq_start);							//   Sets SEQ_START=seq_start.
			errors+=CHAN_setByte(IO24_NUM,seq_end);								//   Sets SEQ_END=seq_end.
		}
		else if (!(myRXBuffer[2]&0x01)&&(myLastInput&0x01)) { 					//  else if switch1 is released :
			 myLastInput=myLastInput&0b11111110;								//   Acknowledges : prevents repetition while reading inputs.
		 }
		getch_start();															//
		c=getch_try();															//  Gets the last character typed without stopping program.
		getch_stop();															//  (see FUNCTIONS above)
		if (c=='e') {															//  if 'e' is typed 
			printf("Requested frequency fs (Hz) : \n");
			scanf("%f",&f_requested);
			FFFFseqUserVal=0;													//
			diff=100.0;															//   Initializes FFFFseqUserVal and inc_time.
			while (diff>10.0||(FFFFseqUserVal<0xFF)) {								//   While 0xFFFFseqUserVal<0xFF and diff>10.0
				if(inc_time++==255) {											// 	  Increases inc_time.								
					if (presca>(1.0/256.0)) presca/=2.0;						//    If inc_time=255 Increases prescale.
				}
				FFFFseqUserVal=((presca*f_internal)/(f_requested*(inc_time+1)));//    calculates 0xFFFFseqUserVal as a function of inc_time and presca
				diff=fabs(((f_internal*presca)/((float)FFFFseqUserVal*(inc_time+1)))
						-(f_requested));										//    calculates diff, the difference between requested frequency and calculated frequency
				printf("diff = %f\t",diff);										//
				printf("inc_time : %d \t",inc_time);							//	  Displays intermediate calculations.	
				printf("presca : %f \t",presca);								//
				printf("FFFFseqUserVal = 0x%04X\n",FFFFseqUserVal);				//
				
			}
			printf("result : \n");												//
			printf("diff = %f\n",diff);											//
			printf("inc_time : %d \n",inc_time);								//
			printf("presca : %f \n",presca);									//
			printf("FFFFseqUserVal = 0x%04X\n",FFFFseqUserVal);					//
			seqUserVal=0xFFFF-FFFFseqUserVal;									//
			printf("seqUserVal 0x%4X \n",seqUserVal);							//   Displays result.
			seqUserVal_l=seqUserVal&0xFF;										//
			seqUserVal_h=seqUserVal>>8&0xFF;									//	 Splits seqUserVal.
			    		
			errors+=CHAN_addr(IO24_NUM,0x148);									//	 Sets current address at 0x148.		//			 
			errors+=CHAN_setByte(IO24_NUM,seqUserVal_l);           				//	 Sets seqUserVal_L=seqUserVal_l.	// 
			errors+=CHAN_setByte(IO24_NUM,seqUserVal_h);						// 	 Sets seqUserVal_H=seqUserVal_h.	//								
			if (presca==0.5){
				errors+=CHAN_setByte(IO24_NUM,(uint8_t)presca);						//	 Sets PRESCA=presca.				// (re)initializes Timer0 with calculated value.
			}
			errors+=CHAN_command(IO24_NUM,0x03);								//   Executes IO24 command 0x03 named SEQUENCER_RUN : restarts sequenced outputs mode because SEQ_USER_VAL and SEQ_PRESCA should have been modified. 
			
			errors+=CHAN_addr(IO24_NUM,0x144);									//   Sets current address at 0x144.
			errors+=CHAN_setByte(IO24_NUM,inc_time);							// 										// (re)initializes Seq sequence settings with calculated value.
			
			printf("Sequencers Seq and Seq1 steps frequency fs is set such as fs=%fHz => Ts=1/fs=%fs.\n\nPress switch 1 to change sequence, type 'e' to edit frequency or 'q' to exit.\n",f_requested,1/f_requested);
			
		}
		else if (c=='q') exit_loop =1;											// else if 'q' is typed exits loop.
	}	
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											EXITS PROGRAM				                                                                                                     															 	    *
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
