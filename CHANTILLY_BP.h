#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
// shared memory
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
//
// ##############################################
// Chantilly IO low-level functions
// ##############################################
#define _CORE_ACCESS_VERSION	0x1e9
#define _CORE_ID				0x1ea
#define _CORE_FIRMWARE_VERSION	0x1eb
//
typedef struct {
	uint8_t core_access_v[7];
	uint8_t core_id[7];
	uint8_t core_firmware_v[7];
} bus_struct;
//
#define _DEBUG_CHANTILLY_OPEN
//
void			CHAN_FN_reset(void);																				// @@@ hard reset,0xBB
unsigned long	dcount(unsigned long howmuch);																		// opt avoider
//
//
int32_t			CHAN_setup(char* app_name,uint8_t hot_stop);																			// memmap,io as in/outs, check for 0xBB connected
int32_t			CHAN_close(void);																					// close memmap,GPIOs as inputs.
void			CHAN_select(unsigned char addr);// subfunction for followings :
//
// IMMEDIATE ACCESS ROUTINES
// uses a source* for multiple byte write writes
//
//
uint32_t	CHAN_setByte(unsigned char corenum,unsigned char what);														// writes 1 byte to the core
uint32_t	CHAN_setBytes(uint8_t corenum,uint16_t max_numbytes,uint8_t* source);
void		CHAN_setBytesAt(unsigned char corenum,unsigned char addr_mod4,unsigned char* source,unsigned int pr_len);		// writes n bytes from {source} to the core (uses CHAN_writeBYTE() functions )
// 
uint32_t	CHAN_getBytes(unsigned char corenum,unsigned int max_numbytes,uint8_t* target);	
//
uint32_t	CHAN_addr4(unsigned char corenum,unsigned char what);				// sets selected CORE's internal ADDR pointer , :4
uint32_t	CHAN_addr(uint8_t core_addr,uint16_t precise);						// sets core pointer to EXACT address	:: slower then CHAN_setaddr()
//
uint32_t	CHAN_command(uint8_t core_addr,uint8_t command_code);					// executes CHAN command for selected address
// helpers
void		CHAN_checkBUS(bus_struct* target,int8_t single_addr,uint8_t display);
uint32_t	CHAN_setFloat(uint8_t corenum,float f);																		// writes 1 float{arm f 32bits} to the core{corenum} (uses CHAN_writeBYTE() functions )
float		CHAN_getFLOATfrom(uint8_t* p);
uint16_t	CHAN_getADCfrom(uint8_t* p);
