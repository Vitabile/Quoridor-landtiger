/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <stdio.h>
#include "lpc17xx.h"
#include "timer.h"
#include "../GLCD/GLCD.h" 


/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/


extern Quoridor q;
extern PossibleWall pw;
extern int J_down;
extern int J_up;
extern int J_left;
extern int J_right;

void TIMER0_IRQHandler (void)
{	
	// show time
	char time_in_char[2];
	sprintf(time_in_char, "%2d", q.time_left);
	GUI_Text(110, 282, (uint8_t *) time_in_char, TEXT, BACKGROUND);
	q.time_left--;
	
	// reset joystick handlers if the time is over
	if (q.time_left == 255){
		J_left = 0;
		J_up = 0;
		J_down = 0;
		J_right = 0;
		if (pw.trap){
			// clear trap message
			LCD_Fill(5,242,45,260,BACKGROUND);
		}
	}
	
  LPC_TIM0->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
