// ###################################################
// #                                                 #
// #         LOCAL CONFIG ( compile time )           #
// #                                                 #
// ###################################################
#define compile_for_pi													// uncomment for PI , comment for asus
#define _IO_PI_MODEL_B_PLUS
//#define _IO_PI_MODEL_B												// legacy, mod_B_PLUS cant use that model
//#define _DEBUG_CHANTILLY_WRITES_LEVEL_1								// level one, moderate debug
//#define _DEBUG_CHANTILLY_WRITES_LEVEL_2								// level two, full debug (write ack times, etc..)
//#define _DEBUG_CHANTILLY_READS_LEVEL_1 								//(not into code)
//#define _DEBUG_CHANTILLY_READS_LEVEL_2								// level two, fulle debug (read ack times 'n erros etc)
//#define _DEBUG_CHC_LOADER												// debug for CHAN_connectUPLOAD() function
#define _NO_DESIGN														// design mode uses labels name arrays

//
// ###################################################
// #              Hardware IO includes               #
// ###################################################
//
#define _IO_USE_BARRIERS_ARMV6
//
#ifdef _IO_USE_BARRIERS_ARMV6
				#include "../common/CHAN/dmb.h"
#endif
#ifdef _IO_PI_MODEL_B_PLUS
		//#include "../common/CHAN/slave_utils.h"							// nanoload,nanosave,nanolist  functions
		
		#include "../common/CHAN/CHAN_IO_BP.h"
		#include "../common/CHAN/CHAN_FUNCTIONS_BP.h"						// holds the GLOBAL defines for C code
		//#include "../common/CHAN/slave_utils.c"
#endif
//
// ###################################################
// #                                                 #
// #                END LOCAL CONFIG                 #
// #                                                 #
// ###################################################
