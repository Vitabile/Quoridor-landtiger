/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "timer/timer.h"
#include "RIT/RIT.h"
#include "button_EXINT/button.h"
#include "Joystick/joystick.h"
#include "GLCD/GLCD.h" 
#include "game.h"

/* for simulator */
#define SIMULATOR 1

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif


// flag for game_mode or wait_mode
volatile char game_mode = 0;

int main(void)
{
	
  SystemInit();  												/* System Initialization (i.e., PLL)  */
	BUTTON_init();												/* BUTTON Initialization              */
  LCD_Initialization();									/* LCD Initialization */
	joystick_init();											/* Joystick Initialization */

	
	// rit for buttons and joystick
	init_RIT(0x004C4B40);									 /* RIT -> 50 ms, RIT set at 100MHZ */
	
	// timer 0 for turn 
	init_timer(0,0x17D7840);               /* 1 sec */
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);	
	
	
	enable_RIT();
	// show start screen
	view_start_screen();
  while (1)	
  {
		// game mode (when press INT0)
		if (game_mode){
			// start a game
			play();
    }
		// low power-mode if we are not in game mode
		else{
			__ASM("wfi");
		}
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
