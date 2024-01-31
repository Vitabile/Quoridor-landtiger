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
#include "../CAN/can.h"


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
extern char J_upLeft;
extern char J_upRight;
extern char J_downLeft;
extern char J_downRight;

void TIMER0_IRQHandler (void)
{	
	//decrement and visualize count
	q.time_left--;
	LCD_Time(q.time_left);
	
	
	// reset joystick handlers if the time is over
	if (q.time_left == 0){
		J_left = 0;
		J_up = 0;
		J_down = 0;
		J_right = 0;
		J_upLeft = 0;
		J_upRight = 0;
		J_downLeft = 0;
		J_downRight = 0;
		if (pw.trap){
			// clear trap message
			LCD_Fill(5,242,150,260,BACKGROUND);
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
	LCD_Fill(20,235,310,310,BACKGROUND);
	GUI_Text(30,240,(uint8_t *) "   None is connected!", NOT_VALID,BACKGROUND);
	GUI_Text(30,260,(uint8_t *) "  Press again INT0 for", NOT_VALID,BACKGROUND);
	GUI_Text(30,280,(uint8_t *) "   a new connection!", NOT_VALID,BACKGROUND);
	reset_connection();
	reset_timer(1);
	disable_timer(1);
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
