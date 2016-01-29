 /*******************************************************************
 *                                                                  *  
 * Chantilly 0x10 - IO24 : Source code for application note AN120   *
 *                                                     			    *
 ****************************************************************************************************************************************************************************************************************************************************
 * CIRCUIT DESCRIPTION :                                                                                                                                                                           													*
 * Core module IO24 is set at number 1 on chantilly stack (redefine IO24_NUM according to hardware-address to change it : between 1 included and 6 included).																						*
 * As described in file chantilly_0x10_AN120_pcb2rev1.PDF, servo1, servo2, servo3 and switch2 are wired respectively on OutputB bit5, bit6 and bit7  and InputA bit1.                                  												*
 * 						            																																																				*
 *  																																																												*
 * PROGRAM DESCRIPTION : Test using IO24 direct core access mode with (immediate inputs and) servo outputs mode.																																	*
 * Applies predefined values at the 3 servos.																																																		*
 * lets the user selects a servo by pressing on switch 2 and changes its pwm value by typing +/-. 																																					*
 * exits if 'q' is typed.																																																							*
 * resets servos to predefined values when exiting.                                                                                                                                                													*
 *  																																																												*
 * 																																																													*
 * Commented lines mainly refers to chantilly use.												                                                																									*
 ***************************************************************************************************************************************************************************************************************************************************/

#define IO24_NUM 1 									// IO24 is core number 1 on chantilly stack. (Choosen by user but must match hardware-address on IO24 Board)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

unsigned char getch_try(void)       				// Gets last character typed without stopping program.                                     
{
	int c=0;
	while (kbhit()) { c=getchar(); }
	return (unsigned char) c;						// One char from queue only
}	


 /***************************************************************************************************************************************************************************************************************************************************
 *  MAIN PROGRAM                                                                                                                                                                                                                                    *
 ***************************************************************************************************************************************************************************************************************************************************/
 
 
int main (int argc,char *argv[ ]) {
	
	bus_struct chantilly_stack;
	uint8_t myRXBuffer[1024];
	uint32_t errors=0;
	int8_t exit_loop=0;
	uint8_t c=0,n=0;
	uint8_t myLastInput=0;
	uint8_t lastn=0;
	uint8_t pwm[4]={0,0x30,0x30,0xfe};				// Initial values for PWMs.
	uint8_t lastPwm[4]={0,0x40,0x60,0x40};
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP CHANTILLY 				                                                                                                  																	*
	************************************************************************************************************************************************************************************************************************************************/
	
	
	if(CHAN_setup("APP100",1)!=0) { printf("Error opening chantilly , error or processID { }\n"); CHAN_close();return 0;}	// stops here if can't open immediate core access mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											CHECKS CHANTILLY STACK 				                                                                                              																	*
	************************************************************************************************************************************************************************************************************************************************/
	
	
	CHAN_checkBUS(&chantilly_stack,1,1);			// Checks what's on the addr1 of the chantilly stack bus.
	CHAN_checkBUS(&chantilly_stack,2,1);			// Checks what's on the addr2 of the chantilly stack bus.
	
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											SETS UP SERVO OUTPUTS MODE				                                                                                                                                                                    *
    ************************************************************************************************************************************************************************************************************************************************/
	
	 
	errors+=CHAN_addr4(IO24_NUM,0x45);          	// Sets current address to 0x45*4=0x114 into IO24  
	errors+=CHAN_setBytes(IO24_NUM,4,pwm);			// Writes PWMs values (stored in pwm which has been filled when initialized) at current address 0x114 into IO24.					 															 
													// only PWM15_VAL, PWM14_VAL and PWM13_VAL are used as PWMs respectively on bit7, bit6 and bit5 of OutputB.
					
	errors+=CHAN_addr(IO24_NUM,0x119);				// Sets current address to 0x119.
	errors+=CHAN_setByte(IO24_NUM,0);				// Writes 0 at current address to inhibit sequencer of servo outputs mode.
	
		
	/************************************************************************************************************************************************************************************************************************************************
	*											STARTS SERVO OUTPUTS MODE				                                                                                                                                                                    *
    ************************************************************************************************************************************************************************************************************************************************/
	
	
	errors+=CHAN_command(IO24_NUM,5);          	 	// Executes IO24 command 0x05 named SERVO_RUN : Starts IO24 (immediate inputs and) servo outputs mode.
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											LOOPS : DRIVES SERVOS BY PWMS		     		                                                                                                                                                    *
    ************************************************************************************************************************************************************************************************************************************************/
	
	
	n=1;
	while (!exit_loop) {									// Entering loop
		getch_stop();										//  
		getch_start();										//  
		c=getch_try();										//  Gets the last character typed without stopping program (see FUNCTIONS above).
		getch_stop();
		if (c==43) pwm[n]=(pwm[n]<0xF8)?pwm[n]+8:0xFF;		//  if pwm[n]<0xF8, pwm[n]+=8 else pwm[n]=0xFF. 
		else if (c==45) pwm[n]=(pwm[n]>0)?pwm[n]-8:0;		//  else if pwm[n]>0, pwm[n]-=8 else pwm[n]=0.
		if (lastPwm[n]!=pwm[n] || lastn!=n) {
			printf("SERVO %d = 0x%2X, (to change pwm value type +/-, to exit type 'q', press switch2 to change active servo).\n",n,pwm[n]);
			lastPwm[n]=pwm[n];
			lastn=n;
		}
		errors+=CHAN_addr4(IO24_NUM,0x45);      // Sets current address to 0x45*4=0x114 into IO24  
		errors+=CHAN_setByte(IO24_NUM,0x00);    // Writes 0x00 at current address 0x114 into IO24 (empty writing to increase current address).
		errors+=CHAN_setByte(IO24_NUM,pwm[1]);	// Writes pwm[0] at current address 0x115 into IO24 (current address increases automatically).
		errors+=CHAN_setByte(IO24_NUM,pwm[2]);  // Writes pwm[1] at current address 0x116 into IO24 (current address increases automatically).
		errors+=CHAN_setByte(IO24_NUM,pwm[3]);	// Writes pwm[2] at current address 0x117 into IO24 (current address increases automatically).
		if (c=='q') exit_loop=1;	
		errors+=CHAN_addr4(IO24_NUM,0x40);
		errors+=CHAN_getBytes(IO24_NUM,3,myRXBuffer);
		if (!(myRXBuffer[2]&0x2)&&(myLastInput&0x02)) {			// if switch2 is pressed (switch2 is set as negative logic input).
			myLastInput=myLastInput&0b11111101;					//  acknowledges : prevents repetition while reading inputs.
			n=(n==3)?1:n+1;										// 	change active servo from servo1 included to servo3 included.
		}
		else if ((myRXBuffer[2]&0x02)&&!(myLastInput&0x02)) myLastInput=myLastInput|0b00000010; // if switch2 is released acknowledges : prevents repetition while reading inputs.
	}
	getch_stop();
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											RESETS AND APPLIES PWM VALUES : MOVES SERVOS TO INITIAL POSITION                                                                                                                                    *
    ************************************************************************************************************************************************************************************************************************************************/

	
	errors+=CHAN_addr4(IO24_NUM,0x45);			// Sets current address to 0x45*4=0x114 into IO24  						 															 
	errors+=CHAN_setByte(IO24_NUM,0x00);        // Writes 0x00 at current address 0x114 into IO24 (empty writing to increase curent address).
	errors+=CHAN_setByte(IO24_NUM,0x40);		// Writes 0x40 at current address 0x115 into IO24 (current address increases automatically).
	errors+=CHAN_setByte(IO24_NUM,0x60);        // Writes 0x60 at current address 0x116 into IO24 (current address increases automatically).
	errors+=CHAN_setByte(IO24_NUM,0x40);		// Writes 0x40 at current address 0x117 into IO24 (current address increases automatically)./**/
	
	
	/************************************************************************************************************************************************************************************************************************************************
	*											EXITS PROGRAM				                                                                                                                                                                        *
    ************************************************************************************************************************************************************************************************************************************************/
	
	
	/** STOPS IO24 */
	 
	sleep(1);										// Lets time to servo to reach requested position before stopping. 
	
	errors+=CHAN_command(IO24_NUM,6);          		// Executes IO24 command 0x06 named STOP : Stops IO24 running mode (optionnal, if not executed IO24 will go on maintening mode
													//																				actually running after having closed chantilly and program).  
	
	/** CLOSES CHANTILLY */
	
	errors+=CHAN_close();							// Closes immediate core access mode.
	
	
	/** DEBUG */
	
	printf("\naccess errors : %d\n\n",errors);		// Displays access errors (optionnal for debug).
	fflush(stdout);
	
	return 0;
	
}
