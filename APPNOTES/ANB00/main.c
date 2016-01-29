/***********************************************************************
 *                                                                     *  
 * Chantilly 0xBB - Bus Board : Code source for application note ANB00 *
 *                                                                     *
 ***************************************************************************************************
 *                                                                                                 *
 * test reading the power supply voltage 														   *
 * 																								   *
 * Core module Bus Board is always at address 7 and use immediate core access mode in this example *
 * Only commented lines affect chantilly use.												       *
 * 																								   *  
 ***************************************************************************************************/


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <CHANTILLY_BP.h>
//
#define EXTVIN_GAIN 0.124668435
#define EXTVIN_STEP 0.004882813
#define DC3V3_GAIN	0.5
#define DC3V3_STEP 	0.004882813
// 
//
int main (int argc,char *argv[ ]) {
	//
	bus_struct	my_bus;
	int32_t res;
	uint32_t errors=0;
	uint8_t myRXBuffer[1024];
	uint8_t i=0;
	float extvin=0;
	float dc3v3=0;
	//
	res=CHAN_setup("ANB00",1);														// open immediate core access mode.
	if (res>0) 																		// if failure
	{ 																				// |	
		printf("Error, used by PID {%d}\n",res);									// |	close immediate core access mode.
		res=CHAN_close();															// |	exit
		return 0;																	// |	
	}																				// |
	if (res==0) 																	// else
	{	
		printf("Open : OK\n");
		i=0;
		while (i<10) {	
			printf ("\n%d remaining time(s).\n",10-i);
			CHAN_addr4(0x07,0x40);													// set ram pointer at core address 7 (Bus Board is allways at address 7) and offset 0x100=4*0x40 (_BB_EXTVIN_VOLTS_L)  
			//																		// CHAN_addr4(0x07,0x40) can be replace by CHAN_addr(0x07,0x100) which is easier but slower to use;
			errors+=CHAN_getBytes(0x07,32,myRXBuffer);								// reads 32 bytes from where the ram pointer has been set (0x100 in this example) and writes them in myRXBuffer. 
			extvin=(float)CHAN_getADCfrom(&myRXBuffer[0])*EXTVIN_STEP/EXTVIN_GAIN;  // myRXBuffer[0] holds the value stored at offset 0x100 in Bus Board.
			dc3v3=(float)CHAN_getADCfrom(&myRXBuffer[4])*DC3V3_STEP/DC3V3_GAIN;     // myRXBuffer[4] holds the value stored at offset 0x104 (_BB_HOST_3V3_L) in Bus Board.
			printf("Main VDC in (ADC 1024) is %f Volts\n",extvin);					//	
				//(float)CHAN_getADCfrom(&myRXBuffer[0])*EXTVIN_STEP/EXTVIN_GAIN);	// 
			printf("3.3 VDC (ADC 1024) is %f Volts\n ",dc3v3);						//			
				//(float)CHAN_getADCfrom(&myRXBuffer[4])*DC3V3_STEP/DC3V3_GAIN);	// 													
			sleep(1);
			i++;
		}
	}
	//
	printf("\naccess errors : %d\n\n",errors);											// report access core errors
	//
	fflush(stdout);
	//
	CHAN_close();																	// close immediate core access mode.
	//
	return 0;
	//
}
