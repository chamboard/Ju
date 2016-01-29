 /*******************************************************************
 *                                                                  * 
 * Chantilly 0x10 - IO24 : Source code for application note AN105   *
 *                                                     			    *
 ****************************************************************************************************************************************************************************************************************************************************
 * CIRCUIT DESCRIPTION : 																																			                                												*
 * Core module IO24 is set at number 1 on chantilly stack (redefine IO24_NUM according to hardware-address to change it : between 1 included and 6 included).	                                    												*
 * As described in file chantilly_0x10_AN105_pcb2rev1.PDF, relay1, relay2, relay3, relay4, switch1 and switch2 are wired respectively on OutputA bit0, bit1, bit2 and bit3 and InputA bit0 and bit1.       											*
 *      																																																											*
 * 																																																													*
 * PROGRAM DESCRIPTION : Test using IO24 direct core access mode with (immediate inputs and) sequenced outputs mode 0.																																*
 * Sequencer Seq used as single sequencer plays a sequence on OutputA. In this particuler case only one sequence is required.																																																		*
 * The user can change the frequency of the sequence if switch1 is pressed the fourth first times or exit program the fifth time.                                                                 													*
 * The sequence is inverted if switch2 is pressed before switch1 has been pressed the fifth time. 												                                                        												*
 *                                       									                  													                                                   													*
 * 		          						                                                   																																							*
 *																																																													*
 * Commented lines mainly refers to chantilly use.												                              					                												                                    *
 ***************************************************************************************************************************************************************************************************************************************************/
 
#define IO24_NUM 1									// IO24 is core number 1 on chantilly stack. (Choosen by user but must match hardware-address on IO24 Board)
 
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <CHANTILLY_BP.h>
  
 
 /***************************************************************************************************************************************************************************************************************************************************
 *  MAIN PROGRAM                                                                                                                                                                                    												*
 ***************************************************************************************************************************************************************************************************************************************************/
 
 
int main (int argc,char *argv[ ]) {
	
	bus_struct chantilly_stack;
	uint8_t myRXBuffer[1024];
	uint8_t myTXBuffer[1024]={1,2,4,8}; 			// the sequence to write to Output A. 
	uint8_t exit_loop=0;
	uint8_t i=0;
	uint8_t myLastInput=1;
	uint8_t myLastInput2=0;
	uint32_t errors=0;
	uint8_t seq_step=1;
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP CHANTILLY 				                                                                                                  																    *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	if(CHAN_setup("APP100",1)!=0) { printf("Error opening chantilly , error or processID { }\n"); CHAN_close();return 0;}	// Stops here if can't open immediate core access mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											CHECKS CHANTILLY STACK 				                                                                                              																    *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	CHAN_checkBUS(&chantilly_stack,1,1);			// Checks what's on the addr1 of the chantilly stack bus
	CHAN_checkBUS(&chantilly_stack,2,1);			// Checks what's on the addr2 of the chantilly stack bus
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP UPDATE PERIOD OF SEQUENCED OUTPUTS MODE 				                                                                                                      								*
	************************************************************************************************************************************************************************************************************************************************/
	
	
	errors+=CHAN_addr4(IO24_NUM,0x52);             	// Sets current address at 0x52*4=0x148 into IO24  (CHAN_addr4() can only access multiple of 4 offsets,
													// 													must do empty reading or writing to increase current address). 
	errors+=CHAN_setByte(IO24_NUM,0x08);           	// Writes 0x08 at current address 0x148 into IO24 (current address increases automatically).
	errors+=CHAN_setByte(IO24_NUM,0xCF);			// Writes 0xCF at current address 0x149 into IO24 (current address increases automatically).  
													// Two previous values at addresses 0x148 and 0x149 are used to set SEQ_USER_VAL=0xCF08 => 0xFFFF-SEQ_USER_VAL=0x30F7=12535. 
	errors+=CHAN_setByte(IO24_NUM,0x01);			// Writes 0x01 at current address 0x14A into IO24.
													// Previous value is used to set SEQ_USER_PRESCA=1 => PRESCAseq=1/4 (0,1,2,..,7 respectively corresponding to 1/2,1/4,1/8,..,1/256). 
	// Sequenced outputs mode is now set with update period, Tseq, such as Tseq=(0xFFFF-SEQ_USER_VAL)*100ns/PRESCAseq=12535*100ns*4=5.014ms.
	// That means, inputs are read every Tseq=5.014ms and sequenced outputs are updated every (SEQ_ASTEP_INC_TIME+1)*Tseq=(SEQ_ASTEP_INC_TIME+1)*5.014ms. 
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP SEQUENCED OUTPUTS MODE				                                                                                                                                                        *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	/** SETS UP SEQUENCED OUTPUTS */		
		
	errors+=CHAN_addr4(IO24_NUM,0x40);				// Sets current address at 0x40*4=0x100 into IO24.
	errors+=CHAN_setByte(IO24_NUM,0);				// Writes 0 at current address 0x100 into IO24. Used to set OUTPUT_A=0. 
	errors+=CHAN_setByte(IO24_NUM,0);				// Writes 0 at current address 0x101 into IO24. Used to set OUTPUT_B=0. 
	errors+=CHAN_addr4(IO24_NUM,0x54);				// Sets current address at 0x54*4=0x150 into IO24.
	errors+=CHAN_setByte(IO24_NUM,0xFF);			// Writes 0xFF at current address 0x150 into IO24. Used to set SEQ_MASK=0xFF. 
	errors+=CHAN_setByte(IO24_NUM,0);				// Writes 0 at current address 0x151 into IO24. Used to set SEQ1_MASK=0. 
	// Since mode of sequencer is 0, Seq plays on OutputA such as OutputA=(data[SEQ_CURSOR]&SEQ_MASK)|OUTPUT_A) with data[X]:value stored at address X and 0x04≤X≤0xFF.
	//						         Seq1 plays on OutputB such as OutputB=(data[SEQ_CURSOR+128]&SEQ1_MASK)|OUTPUT_B)=OUTPUT_B as SEQ1_MASK=0.
	// With previous settings, Seq plays on OutputA and Seq1 is unused so OutputB=OUTPUT_B same as in immediate outputs mode.   
	
	
	/** WRITES Seq SEQUENCE VALUES */
	
	errors+=CHAN_addr4(IO24_NUM,0x01);				// Sets current address at 0x04 into IO24.							
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Writes Seq sequence (stored in myTXBuffer which has been filled when initialized) at current address 0x04 into IO24.
	// Using mode 0 (for single sequencer Seq) implies that Seq sequence must be stored between included addresses 0x04 and 0x7F in RAM.
	
	
	/** SETS UP SEQUENCER Seq SETTINGS */
	
													// Presets via myTXBuffer :
	myTXBuffer[0]=seq_step;   						// SEQ_STEP=seq_step=1 : sequencer Seq increases.	
	myTXBuffer[1]=0x04;								// SEQ_CURSOR=0x04 : 	 current RAM address of Seq sequence (permanently overwritten but as Seq increase must be initialized such as SEQ_CURSOR≤SEQ_END).
	myTXBuffer[2]=0x04;								// SEQ_START=0x04 : 	 RAM address where Seq sequence begins.  
	myTXBuffer[3]=0x07;   							// SEQ_END=0x0B : 		 RAM address where Seq sequence ends. In mode 0, 0x04≤SEQ_START≤SEQ_END≤0x7F.		//
	myTXBuffer[4]=15;								// SEQ_ASTEP_INC_TIME :  used to change delay Ts between steps of Seq sequence such as Ts=(SEQ_ASTEP_INC_TIME+1)*Tseq=16*5.014ms=80.204ms. 
	myTXBuffer[5]=0;								/*********************************************non implementé ??????*************************************/
	myTXBuffer[6]=1;								// SEQ_WRAP=1 : 		 plays sequences infinitely by setting SEQ_CURSOR=SEQ_START when Seq sequence ends.
	myTXBuffer[7]=0;								// SEQ_MODE=0 : 		 mode 0 to use single sequencer Seq.
	// All eight previous parameters can be edited without having to stop running mode. They take effects when executing IO24 command 0x03 named SEQUENCER_RUN.
	errors+=CHAN_addr4(IO24_NUM,0x50);				// Sets current address at 0x50*4=0x140 into core IO24.
	errors+=CHAN_setBytes(IO24_NUM,8,myTXBuffer);	// Writes sequence parameters (stored in mtTXBuffer) in RAM at current address 0x140.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											STARTS SEQUENCED OUTPUTS MODE																																										*
	************************************************************************************************************************************************************************************************************************************************/											

	
	errors+=CHAN_command(IO24_NUM,0x03);			// Executes IO24 command 0x03 named SEQUENCER_RUN : Starts IO24 (immediate inputs and) sequenced outputs mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											LOOPS :			             	                                                                                                                                                                    *
	************************************************************************************************************************************************************************************************************************************************/
	
	
	printf("\nIO24 sequenced outputs mode 0 is started : \nSequencers Seq and Seq1 are running with \tsteps delay Ts set such as Ts=80.204ms \n\n");
	printf("\tPress switch1 to double steps delay Ts, press switch2 to invert sequence way. The fifth time switch1 is pressed, program exits.\n");  
	i=0;
	while (!exit_loop) {									// Entering loop () :
		errors+=CHAN_addr4(IO24_NUM,0x40);
		errors+=CHAN_getBytes(IO24_NUM,3,myRXBuffer);
		if ((myRXBuffer[2]&0x1)&&!myLastInput) {    		//  if switch1 pressed (changes timer0 and stops sequencer the fifth time) :
			myLastInput=1;									//   Acknowledges :prevents repetition while reading inputs.				
			printf("Steps sequence delay is set such as Ts=%fms\n\n",(15*pow(2,i+1)+1)*5.014);
			printf("Press switch1 to double steps delay Ts, press switch2 to invert sequence way. You must press switch1 %d times before program exits.\n",4-i);
			errors+=CHAN_addr4(IO24_NUM,0x51);				//   sets current address at 4*0x51=0x144 into IO24 
			errors+=CHAN_setByte(IO24_NUM,15*pow(2,i+1));		//   Sets SEQ_ASTEP_INC_TIME=15*2^(i+1) (switch1 pressed 1x : 31*5.014ms=155.434 / 2x : 61*5.014ms=305.854ms / 3x : 121*5.014ms=606.694ms / 4x : 241*5.014ms=1208.374ms).
			if (i++==4) exit_loop=1;						//   if it is the fifth time that switch1 is pressed exit loop.						
		}
		else if (!(myRXBuffer[2]&0x1)&&myLastInput) {		//  else if switch1 is released :
			myLastInput=0;									//   Acknowledges: prevents repetition while reading inputs.
		} 
		
		if (!(myRXBuffer[2]&0x2)&&myLastInput2) {			//  if switch2 is pressed (inverts sequence way, switch 2 is set as negative logic input) :
			myLastInput2=0;									//   Acknowledges : prevents repetition while reading inputs.
			if (seq_step==1) seq_step=2;					//	 
			else seq_step=1;								//   Sets seq_step to 1 or 2 switch previous value.
			errors+=CHAN_addr4(IO24_NUM,0x50);				//	 Sets current address at 4*0x50=0x140
			errors+=CHAN_setByte(IO24_NUM,seq_step);		//   Sets SEQ_STEP=seq_step.
			printf("Sequence way has changed\n\n");			
			printf("Press switch1 to double steps delay Ts, press switch2 to invert sequence way. You must press switch1 %d times before program exits.\n",5-i);											
		}
		else if ((myRXBuffer[2]&0x2)&&!myLastInput2) { 		// if switch2 is released
			myLastInput2=1;									//  Acknowledges : prevents repetition while reading inputs.
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
