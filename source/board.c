
/*
*------------------------------------------------------------------------------
* device.c
*
* Board specipic drivers module(BSP)
*
*
* The copyright notice above does not evidence any
* actual or intended publication of such source code.
*
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* Include Files
*------------------------------------------------------------------------------
*/


#include "board.h"
//#include "config.h"
#include "typedefs.h"




/*
*------------------------------------------------------------------------------
* Private Defines
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Macros
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Data Types
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Variables
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Variables (static)
*------------------------------------------------------------------------------
*/

static BOOL ledState;

/*
*------------------------------------------------------------------------------
* Public Constants
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Constants (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Private Function Prototypes (static)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Functions
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* void InitializeBoard(void)

* Summary	: This function configures all i/o pin directions
*
* Input		: None
*
* Output	: None
*
*------------------------------------------------------------------------------
*/
void BRD_init(void)
{
	// set all anolog channels as Digital I/O
	ADCON0 = 0x00;
	ADCON1 = 0x0F;

	// Internal oscillator setup
	OSCCON |= 0X70; 		//internal oscillator 8MHz
	OSCTUNEbits.PLLEN = 1;	//PLL Enable
	
	SER_TX_DIR 		=  0;
	SER_RX_DIR 		=  1;
	
	// Configure keypad input
	KBD_ROW0_DIR = PORT_IN;
	KBD_ROW1_DIR = PORT_IN;
	KBD_ROW2_DIR = PORT_IN;
	KBD_ROW3_DIR = PORT_IN;
	KBD_COL0_DIR = PORT_OUT;
	KBD_COL1_DIR = PORT_OUT;
	KBD_COL2_DIR = PORT_OUT;


	// Rs485 Direction Control
	TX_EN_DIR = PORT_OUT;
	TX_EN = SWITCH_OFF;

	
	// Configure Buzzer output
	BUZ_OP_DIR 			= PORT_OUT;	
	BUZ_OP 				= SWITCH_OFF;

	LAMP_GREEN_DIR		= PORT_OUT;
	LAMP_GREEN 			= SWITCH_OFF;
	
	LAMP_RED_DIR		= PORT_OUT;
	LAMP_RED 			= SWITCH_OFF;

	LAMP_YELLOW_DIR		= PORT_OUT;
	LAMP_YELLOW 		= SWITCH_OFF;
	
	// Configure heart beat LED output 
	HEART_BEAT_DIR = PORT_OUT;
	HEART_BEAT = ~SWITCH_OFF;	
	
	// Configure LCD port as outputs
	LCD_D7_DIR = PORT_OUT;
	LCD_D6_DIR = PORT_OUT;
	LCD_D5_DIR = PORT_OUT;
	LCD_D4_DIR = PORT_OUT;
	LCD_E_DIR  = PORT_OUT;
	LCD_RW_DIR = PORT_OUT;
	LCD_RS_DIR = PORT_OUT;
}



/*
*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

/*
*  End of device.c
*/
