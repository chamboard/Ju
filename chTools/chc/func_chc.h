#ifndef CHANTILLYMANAGER_FUNCTIONS_H
	#define CHANTILLYMANAGER_FUNCTIONS_H


	//#define _COREDEF_0X_H	"/home/pi/code_004/common/CHAN/coredef_0x%X.h"
	//#define _COREDEF_0X_H	"./coresdefs/coredef_0x%X.h"
	#define _COREDEF_0X_H	"/usr/local/bin/chantilly_utils/chc/coredefs/coredef_0x%X.h"
	
	//#define _COREDEF_PATH	"/home/pi/code_004/common/CHAN/coredef%s.h"
	//#define _COREDEF_PATH	"./coresdefs/coredef%s.h"
	#define _COREDEF_PATH	"/usr/local/bin/chantilly_utils/chc/coredefs/coredef%s.h"
	
	//
	#define _THIS_APP_ID 			0xFF
	#define _THIS_VERS 				0x01
	//
	#define _ACCESS_OFFLINE_CORE	0
	#define _ACCESS_ONLINE_CORE		1
	#define _READ 	  				2
	#define _WRITE	  				3
	#define _EXECUTE  				4
	//#define _debug    			7
	//#define _DEBUG
	//

	typedef struct
	{
		uint8_t					func[64];
		uint8_t					txtFunc[64][50];
		uint8_t					type[255];
		int8_t 					label[255][150];
		int16_t					addr0[255];
		int16_t					addr1[255];
		int8_t					txt0[255][100];
		int8_t					txt1[255][100];
		int8_t					format[255][150];
		float					gain[255];
		float					step[255];
		float					high_limit[255];
		float					low_limit[255];
		uint8_t					count_params[255];
		int8_t					params[255][5][100];
		int8_t					onlyRead[255][150];
		uint8_t 				eepromOffset[255];
		uint8_t					value0[255];
		uint8_t					value1[255];
	}pcui;

	uint8_t* strtoupper(uint8_t *s);
	uint8_t *str_sub(uint8_t *s,unsigned int start,unsigned int end);

	void parse_core_list();

	void parse_coredef(uint8_t* id, pcui* ppp);
	//void CHAN_checkBUS2();
	//void CHAN_checkBUS3();
	void list_connected_CORES(bus_struct*);
	void dumpRam_CORE(uint8_t slot,uint8_t coreAddress);
	void dumpRam_CORE_I(uint8_t coreAddress);
	void read_CORE(uint8_t slot,uint8_t core_address,uint16_t ramAddress,uint8_t count,pcui* ppp);
	void read_CORE_I(uint8_t core_address,uint32_t ramAddress,uint8_t count,uint8_t decimal,pcui* ppp);
	void mapRam_CORE(uint8_t slot,uint8_t core_address,pcui* ppp);
	void mapRam_CORE_I(uint8_t core_address,pcui* ppp);
	void mapFunc_CORE(pcui* ppp);
	void write_CORE(uint8_t slot,uint8_t core_address,uint16_t ramAddress,uint8_t val);
	void write_CORE_I(uint8_t core_address,uint16_t ramAddress,uint8_t val);
	void formatWrite_CORE(uint8_t slot,uint8_t core_address,uint16_t ramAddress,uint8_t val_l,uint8_t val_h);
	void formatWrite_CORE_I(uint8_t core_address,uint16_t ramAddress,uint8_t val_l,uint8_t val_h);
	pid_t create_process(void);
	void son_process(char *arg[]);

	#include "./func_chc.c"
#endif
