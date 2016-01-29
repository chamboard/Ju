 /*******************************************************************
 *                                                                  *  
 * Chantilly 0x10 - IO24 : Source code for application note AN100   *
 *                                                     			    *
 ****************************************************************************************************************************************************************************************************************************************************
 * CIRCUIT DESCRIPTION :                                                                                                                                                                            												*
 * Core module IO24 is set at number 1 on chantilly stack (redefine IO24_NUM according to hardware-address to change it : between 1 included and 6 included).																						*
 * As described in file chantilly_0x10_AN100_pcb2rev1.PDF, relay1, relay2, relay3, relay4, switch1 and switch2 are wired respectively on OutputA bit0, bit1, bit2 and bit3 and InputA bit0 and bit1.												*
 *  																																																												*
 *																																																													*
 * PROGRAM DESCRIPTION : Test using IO24 direct core access mode with immediate (inputs/)outputs mode.																																				*
 * Asks which relay to switch every 40.112 ms and how many times to do it. After what, by action on switch1, starts the countdown or pauses it before it finished. 					             													*
 * When countdown finished, program counts rising edges on switch1 and stores result in CounterS, falling edges on switch2 in CounterT, simultaneous edges on switch1 and switch2 in CounterU      													*
 * and simultaneous high levels on switch1 and switch2 in CounterV.                                                                                                                              													*
 * Counters increase each steps of the timer. That means, if both switch1 and switch2 are hold high, CounterU increases at each step of the timer until one of the switch is released.            													*
 * For CounterU, the use of manual switches implies timer step must be long enough to consider the changes as simultaneous.                                                                     													*
 *                                       									                  																																						*
 * Commented lines mainly refers to chantilly use.												                                                                                                 													*
 ***************************************************************************************************************************************************************************************************************************************************/
  
#define IO24_NUM 1 									// IO24 is core number 1 on chantilly stack. (Choosen by user but must match hardware-address on IO24 Board)
 
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
 
#include <CHANTILLY_BP.h>
  
 
 /***************************************************************************************************************************************************************************************************************************************************
 *  MAIN PROGRAM                                                                                                      												                                                                                *
 ***************************************************************************************************************************************************************************************************************************************************/
 
 
int main (int argc,char *argv[ ]) {
	
	bus_struct chantilly_stack;
	uint8_t myRXBuffer[1024];
	uint8_t myTXBuffer[1024];
	uint8_t exit_loop=0;
	uint8_t i=0;
	uint8_t myOutput=0;
	uint8_t myLastInput=0;
	uint8_t myCount_S=0;
	uint8_t myCount_T=0;
	uint8_t myCount_U=0;
	uint8_t myCount_V=0;
	int32_t times;
	uint8_t run=0;
	uint32_t errors=0;
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP CHANTILLY 				                                                                                                  																	*
	************************************************************************************************************************************************************************************************************************************************/
	
	
	if(CHAN_setup("APP100",1)!=0) { printf("Error opening chantilly , error or processID { }\n"); CHAN_close();return 0;}	// Stops here if can't open immediate core access mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											CHECKS CHANTILLY STACK 				                                                                                              																	*
	************************************************************************************************************************************************************************************************************************************************/
	
	
	CHAN_checkBUS(&chantilly_stack,1,1);			// Checks what's on the addr1 of the chantilly stack bus.
	CHAN_checkBUS(&chantilly_stack,2,1);			// Checks what's on the addr2 of the chantilly stack bus.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP UPDATE PERIOD OF IMMEDIATE OUTPUTS MODE 				                                                                                                      								*
	************************************************************************************************************************************************************************************************************************************************/
	
	
	errors+=CHAN_addr(IO24_NUM,0x104);             	// Sets current address to 0x184 into IO24.  
	errors+=CHAN_setByte(IO24_NUM,0x08);           	// Writes 0x08 at current address 0x184 into IO24 (current address increases automatically).
	errors+=CHAN_setByte(IO24_NUM,0xCF);			// Writes 0xCF at current address 0x185 into IO24 (current address increases automatically).  
													// Two previous values at addresses 0x184 and 0x185 are used to set IMM_PERIOD=0xCF08. 
	// Immediate outputs mode is now set with update period Timm such as Timm=(0xFFFF-IMM_PERIOD)*100ns*4=12535*100ns*4=5.014ms.				 
	// That means inputs are read, outputs are written and counters are updated every 5.014 ms when running immediate outputs mode.
				
	
	/************************************************************************************************************************************************************************************************************************************************
	*											STARTS IMMEDIATE OUTPUTS MODE																																										*
	************************************************************************************************************************************************************************************************************************************************/											
	
												
	errors+=CHAN_command(IO24_NUM,0x01);			// Executes IO24 command 0x01 named IMMEDIATE_RUN : Starts IO24 immediate (inputs/)outputs mode.
  	
  	
	/************************************************************************************************************************************************************************************************************************************************
	*											LOOPS FIRST TIME  					                                                                                              																	*
	************************************************************************************************************************************************************************************************************************************************/
	
	
	/** PRESETS LOOP */
	
	do {
		printf ("\nWhich relay do you want activate ? 1,2,3 or 4 (0 to exit) : ");      	// Asks the user which relay to activate ...
		scanf("%1x",(unsigned int*)&myOutput);
	} while (myOutput>4);	
	if (!myOutput) exit_loop=1;
	else {
		do {	
			printf("How many times do you want to activate it ? 1,2,3...20 (0 to exit) : ");// Asks the user how many times to activate the selected relay
			scanf("%d",(int*)&times);
		} while (times>20);
		if (!times)	exit_loop=1;
		if (myOutput&&times) {
			printf("Relay %d will be driven %d times after you will press switch1.\n",myOutput,times);
			printf("%d remaining time(s), Press switch1 to start or pause.\n",times);
		}
		if (myOutput==3) myOutput=4;														//
		else if (myOutput==4) myOutput=8;													// myOutput is used for keyboard seizure so must adjust it to activate selected relay.
	}	
	
	
	/** LOOPS : DRIVES RELAYS IN IMMEDIATE OUTPUTS MODE */
	
	while (i<times&&times) {					// Entering loop :
		
		errors+=CHAN_addr(IO24_NUM,0x102);					//  Sets current address to 0x102 into IO24. 
		errors+=CHAN_getBytes(IO24_NUM,1,myRXBuffer);		//  Reads value of INPUT_A and stores result in myRXBuffer.					
																													
		if (!(myRXBuffer[0]&0x1)&&myLastInput) {			//  If switch1 is released :  		    						                                     						                    									
			myLastInput=0;									// 	 Acknoledges : prevents repetition while reading inputs.   
		}
		else if (myRXBuffer[0]&0x1&&!myLastInput){			//  Else if switch1 is pressed :
			myLastInput=1;									//   Acknoledges : prevents repetition while reading inputs.
			run=1-run;										// 	 Sets run to 0 or 1 switch previous value. 
			if (run) printf("Start\n");
			else printf("Pause\n");
		}	
		if (run) {											// if run :		
			errors+=CHAN_addr(IO24_NUM,0x100);				//  Sets current address to 0x100 into IO24.
			errors+=CHAN_setByte(IO24_NUM,myOutput);		//  Sets OUTPUT_A=myOutput.
			usleep(100000);
			errors+=CHAN_addr(IO24_NUM,0x100);				//  (Re)sets current address 0x100 into IO24
			errors+=CHAN_setByte(IO24_NUM,0); 				//  Sets OUTPUT_A=0.
			usleep(100000);
			printf("%d remaining time(s)\n",times-1-i);
			if (times-1-i==0) printf("End\n\npress switch1 or switch2 to continue.\n\n");
			i++;
		}
	} 
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP LOGIC INPUT POLARITY 				                                                                                      																	*
	************************************************************************************************************************************************************************************************************************************************/
  	
  	
  	myTXBuffer[0]=0b00000010;						// Presets myTXBuffer to set negative loqic for bit1 of Input A.
  	errors+=CHAN_addr(IO24_NUM,0x106);             	// Sets current address to 0x104 into IO24.
  	errors+=CHAN_setByte(IO24_NUM,myTXBuffer[0]);	// Sets INPUT_POL_A=myTXBuffer[0]=0b00000010.
  	//Inverts switch 2 logic polarity which is wired in negative logic in order to have positive logic on both switches 1 and 2 respectively wired on bit0 and bit1 of InputA./**/	
  	
  	
	/************************************************************************************************************************************************************************************************************************************************
	*											(RE)SETS UP UPDATE PERIOD OF IMMEDIATE OUTPUTS MODE  				                                                                                                      										*
	************************************************************************************************************************************************************************************************************************************************/
	
	errors+=CHAN_addr(IO24_NUM,0x104);             	// Sets current address to 0x61*4=0x184 into IO24  (CHAN_addr4() is faster than CHAN_addr() but can only access multiple of 4 addresses,
													// 											The use of CHAN_addr4() can implies empty reading or writing to access intermediate addresses). 
	errors+=CHAN_setByte(IO24_NUM,0xFF);           	// Writes 0x08 at current address 0x184 into IO24 (current address increases automatically).
	errors+=CHAN_setByte(IO24_NUM,0x00);			// Writes 0xCF at current address 0x185 into IO24 (current address increases automatically).  
	// Two previous values at addresses 0x184 and 0x185 are used to set IMM_PERIOD=0x00FF=> Timm=(0XFFFF-IMM_PERIOD)*100ns*1/4=(0xFFFF-0x00FF)*100ns*1/4=0xFF00*100ns*1/4=65280*100ns*1/4=65280=26.112ms 
	// Immediate (inputs/)outputs mode update period is now set with period Timm such as Timm=26.112ms. (Timm is set very long cause to CounterU, see below).				 
	// Inputs are read, outputs are written and counters updated every 26.112 ms.
	 
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP COUNTERS				                                                                                                      																*
	************************************************************************************************************************************************************************************************************************************************/
	
	
	/** SETS COUNTER S */
													// Presets via myTXBuffer :
	myTXBuffer[0]=0b00000011;						// IN_A_MASK_S=0b00000011  	CounterS see events on bit1 and bit0 of InputA.
	myTXBuffer[1]=1;								// IN_A_MODE_S=1  			Events to count by CounterS are rising edges.    
	myTXBuffer[2]=0;								// IN_A_STORE_S=0 			CounterS counts events on any bit1 or bit0 of InputA indifferently of value of IN_A_MATCH_S .
	myTXBuffer[3]=0b11111111;						// IN_A_MATCH_S=0b11111111	(Unused here because IN_A_STORE_S is set to 0). Result : CounterS counts any rising edges on bit1 or bit0 of InputA.
	errors+=CHAN_addr4(IO24_NUM,0x47);				// Sets current address at 0x47*4=0x11C.		 
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Writes CounterS settings (stored in myTXBuffer) at current address.
	
	
	/** SETS COUNTER T */
													// Presets via myTXBuffer :
	myTXBuffer[0]=0b00000011;						// IN_A_MASK_T=0b00000011  	CounterT see events on bit1 and bit0 of InputA.
	myTXBuffer[1]=2;								// IN_A_MODE_T=2  			Events to count by CounterT are falling edges.
	myTXBuffer[2]=1;								// IN_A_STORE_T=1 			CounterT counts simultaneous falling edges on all bits of InputA set to 1 in IN_A_MASK_T and matching with bits set to 1 in IN_A_MATCH_T. 
	myTXBuffer[3]=0b00000010;						// IN_A_MATCH_T=0b00000010 	Result : CounterT counts falling edges on bit1 of Input A.
	errors+=CHAN_addr4(IO24_NUM,0x48);				// Sets current address at 0x48*4=0x120.		
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Writes CounterT settings (stored in myTXBuffer) at current address.
	
	
	/** SETS COUNTER U */
													// Presets via myTXBuffer :
	myTXBuffer[0]=0b00000011;						// IN_A_MASK_U=0b0000011   	CounterU see events on bit1 and bit0 of InputA.
	myTXBuffer[1]=3;								// IN_A_MODE_U=3  			Events to count are both rising and falling edges.
	myTXBuffer[2]=1;								// IN_A_STORE_U=1 			CounterU counts simultaneous falling and rising edges on all bits of InputA set to 1 in IN_A_MASK_U and matching with bits set to 1 in IN_A_MATCH_U. 
	myTXBuffer[3]=0b00000011;						// IN_A_MATCH_U=0b0000011	Result : CounterU counts simultaneous falling and rising edges on bit1 and bit0 of InputA.
	errors+=CHAN_addr4(IO24_NUM,0x49);				//																				
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Writes counter U settings (stored in myTXBuffer) at address 0x49*4=0x124 in RAM.
	
	
	/** SETS COUNTER V */
													// Presets via myTXBuffer :
	myTXBuffer[0]=0b00000011;						// IN_A_MASK_V=0b0000011   	CounterV see events on bit1 and bit0 of InputA.
	myTXBuffer[1]=0;								// IN_A_MODE_V=0  			Events to count are high states (corresponding to a time measure).
	myTXBuffer[2]=1;								// IN_A_STORE_V=1 			CounterV counts simultaneous high states on all bits of InputA set to 1 in IN_A_MASK_V and matching with bits set to 1 in IN_A_MATCH_V.
	myTXBuffer[3]=0b00000011;						// IN_A_MATCH_V=0b0000011	Result : CounterV counts simultaneous high states on bit1 and bit0 of InputA.
	errors+=CHAN_addr4(IO24_NUM,0x4a);				//
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Writes counter V settings (stored in myTXBuffer) at address 0x4a*4=0x128 in RAM.

	
	/** INITIALIZES COUNTERS VALUES */
													// Presets via myTXBuffer :
	myTXBuffer[0]=0;								// IN_A_CPT_S=0;
	myTXBuffer[1]=0;								// IN_A_CPT_T=0;
	myTXBuffer[2]=0;								// IN_A_CPT_U=0;
	myTXBuffer[3]=0;								// IN_A_CPT_V=0;
	errors+=CHAN_addr4(IO24_NUM,0x56);				//
	errors+=CHAN_setBytes(IO24_NUM,4,myTXBuffer);	// Initializes values of counters (via myTXBuffer).
	  	
  	
  	/************************************************************************************************************************************************************************************************************************************************
	*											RESTARTS IMMEDIATE OUTPUTS MODE																																										* 
	************************************************************************************************************************************************************************************************************************************************/
  	
  	
  	errors+=CHAN_command(IO24_NUM,0x01);			// Executes IO24 command 0x01 named IMMEDIATE_RUN : Starts IO24 immediate (inputs/)outputs mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											LOOPS SECOND TIME  					                                                                                              																	*
	************************************************************************************************************************************************************************************************************************************************/
		
	
	/**  LOOPS : COUNTS INPUT STATES */	
	
	while (myCount_V<30&&!exit_loop) {					// Entering loop :
		errors+=CHAN_addr4(IO24_NUM,0x56);				//  Sets current address at 0x56*4=0x158 into IO24
		errors+=CHAN_getBytes(IO24_NUM,4,myRXBuffer);	//  Reads IN_A_CPT_S, IN_A_CPT_T, IN_A_CPT_U, IN_A_CPT_V and stores values into myRXBuffer. 
		if (myCount_S!=myRXBuffer[0] || myCount_T!=myRXBuffer[1] || myCount_U!=myRXBuffer[2] || myCount_V!=myRXBuffer[3]) {  //  if any value of counters has changed :
			myCount_S=myRXBuffer[0];																						 //
			myCount_T=myRXBuffer[1];																						 //   Values of variables are updated with values of counters
			myCount_U=myRXBuffer[2];																						 //   			(Values of counters increased automatically in function	of events on InputA and counter settings).
			myCount_V=myRXBuffer[3];																						 // 			
			printf("number of rising edges on switch1 and switch2 : \t\t\t\tmyCount_S==%d \n",myCount_S);					 //
			printf("number of falling edges on switch2 : \t\t\t\t\t\tmyCount_T==%d \n",myCount_T);							 //
			printf("number of simultaneous falling and rising edges on switch1 and switch2 : \tmyCount_U==%d \n",myCount_U); //
			printf("number of simultaneous high states on switch1 and switch2 : \t\t\tmyCount_V==%d \n",myCount_V);			 //
			printf("To exit, maintain switch1 and switch2 pressed until myCount_V==30.\n\n\n");								 //	  Values of counters are displayed.
		}																													 
	}																														 
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											RESETS LOGIC INPUT POLARITY 				                                                                                      																	*
	************************************************************************************************************************************************************************************************************************************************/

	
	myTXBuffer[0]=0;								// Presets myTXBuffer to set positive loqic for Input A.
  	errors+=CHAN_addr(IO24_NUM,0x106);             	// Sets current address to 0x104 into IO24.
  	errors+=CHAN_setByte(IO24_NUM,myTXBuffer[0]);	// Sets INPUT_POL_A=myTXBuffer[0]=0.
	
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											EXITS PROGRAM				                                                                                                      																	*
	************************************************************************************************************************************************************************************************************************************************/				
		
					
	/** STOPS IO24 */
	 
	errors+=CHAN_command(IO24_NUM,6);          		// Executes IO24 command 0x06 named STOP : Stops IO24 running mode (optionnal, if not executed IO24 will go on maintening mode
													//																				actually running after having closed chantilly and program).  
	
	/** CLOSES CHANTILLY	*/
	
	errors+=CHAN_close();							// Closes immediate core access mode.
	
	
	/** DEBUG */
	
	printf("\naccess errors : %d\n\n",errors);		// Displays access errors (optionnal for debug).
	fflush(stdout);
	
	
	return 0;

}
