#ifndef CONFIG_H
#define CONFIG_H

/*
*------------------------------------------------------------------------------
* config.h
*
*/



#define DEVICE_ADDRESS			1


//#define __FACTORY_CONFIGURATION__



//DIGIT DISPLAY TEST
#define __DISPLAY_TEST__
#define __COMMON_CATHODE__

#define RS485
//#define __ADC_COUNT_DISPLAY__


//COM module configuration
#define __BCC_XOR__
#define __RESPONSE_ENABLED__
//#define __LOOP_BACK__
#define BROADCAST_ADDRESS		0xFF
#define CMD_SOP	0xAA
#define CMD_EOP 0xBB
#define RESP_SOP	0xCC
#define RESP_EOP	0xDD


enum
{
	RX_PACKET_SIZE = 50,
	TX_PACKET_SIZE = 50
};
#define 	RX_PACKET_SIZE		(RX_PACKET_SIZE)	
#define 	TX_PACKET_SIZE		(TX_PACKET_SIZE)


/*
*----------------------------------------
*	Keypad Configurations
*----------------------------------------
*/
//#define __FACTORY_CONFIGURATION__
//#define __SIMULATION__



enum
{
	MAX_INPUT_CHARS = 24,
	MAX_KEYS = 24,
	MAX_CHAR_PER_KEY = 4,
	MIN_KEYPRESS_DURATION = 40 

	
};








/*
*------------------------------------------------------------------------------
* Public Data Types
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Variables (extern)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Constants (extern)
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* Public Function Prototypes (extern)
*------------------------------------------------------------------------------
*/

#endif
/*
*  End of config.h
*/



