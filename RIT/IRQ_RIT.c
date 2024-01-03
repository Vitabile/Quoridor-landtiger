/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../GLCD/GLCD.h" 
#include "../game/game.h"
#include "../Joystick/joystick.h"


/* access at game structures */
extern Quoridor q;
extern PossibleWall pw;
extern PossibleShifts ps;

/* access to main variable */
extern char game_mode;

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/


/* define variable for buttons handlers */
volatile int down_0 = 0;
volatile int down_1 = 0;
volatile int down_2 = 0;

/* define variable for joystick handlers */
volatile int J_down = 0;
volatile int J_up = 0;
volatile int J_left = 0;
volatile int J_right = 0;

void RIT_IRQHandler (void)
{		
		/* game mode */
		if (game_mode){
			
			// joystick up
			if ((LPC_GPIO1->FIOPIN & (1<<29)) == 0){
				// shift mode
				if (q.move_mode == 0){
					J_up++;
					if (J_up==1){
						joystick_player(0);
						J_left = 0;
						J_right = 0;
						J_down = 0;
					}
				}
				// wall mode
				else if(q.move_mode && pw.position[1] != 0){
					restore_walls();
					pw.position[1] -= 1;
					pw.valid = wall_check();
				}
			}
			
			// joystick down
			if ((LPC_GPIO1->FIOPIN & (1<<26)) == 0){
				// shift mode
				if (q.move_mode == 0){
					J_down++;
					if (J_down==1){
						joystick_player(1);
						J_left = 0;
						J_right = 0;
						J_up = 0;
					}
				}
				// wall mode
				else if(q.move_mode && pw.position[1] != 5){
					restore_walls();
					pw.position[1] += 1;
					pw.valid = wall_check();
				}
			}
			
			// joystick left
			if ((LPC_GPIO1->FIOPIN & (1<<27)) == 0){
				// shift mode
				if (q.move_mode == 0){
					J_left++;
					if (J_left==1){
						joystick_player(2);
						J_up = 0;
						J_right = 0;
						J_down = 0;
					}
				}
				// wall mode
				else if(q.move_mode && pw.position[0] != 0){
					restore_walls();
					pw.position[0] -= 1;
					pw.valid = wall_check();
				}
			}
			
			// joystick right
			if ((LPC_GPIO1->FIOPIN & (1<<28)) == 0){
				// shift mode
				if (q.move_mode == 0){
					J_right++;
					if (J_right==1){
						joystick_player(3);
						J_left = 0;
						J_up = 0;
						J_down = 0;
					}
				}
				// wall mode
				else if(q.move_mode && pw.position[0] != 5){
					restore_walls();
					pw.position[0] += 1;
					pw.valid = wall_check();
				}
			}
			
			// joystick select
			if ((LPC_GPIO1->FIOPIN & (1<<25)) == 0){
			
				// wall mode
				if(q.move_mode){
					if (pw.valid){
						joystick_confirm_wall(pw.position[0],pw.position[1],pw.orientation);
						q.move_mode = 0;
						pw.orientation = 1;
						pw.position[0] = 3;
						pw.position[1] = 2;
						pw.valid = 0;
					}
				}
				// shift mode
				else{
					if (J_up){
						joystick_confirm(0);
					}
					else if(J_down){
						joystick_confirm(1);
					}
					else if(J_left){
						joystick_confirm(2);
					}
					else if(J_right){
						joystick_confirm(3);
					}
				}
				J_left = 0;
				J_up = 0;
				J_down = 0;
				J_right = 0;
			}
			
			// button INT1
			if(down_1!=0){  
			down_1 ++;  
			if((LPC_GPIO2->FIOPIN & (1<<11)) == 0){
				switch(down_1){
				case 2:
					//active or disable wall_mode
					q.move_mode += 1;
					q.move_mode %= 2;
					// wall mode after change
					if (q.move_mode){
						if (q.remaining_walls[q.turn_index]){
							clear_possible_shifts();
							pw.valid = wall_check();
						}
						else{
							GUI_Text(5, 245 , (uint8_t *) "No walls, move the token.", Red, White);
							q.move_mode = 0;
						}
					}
					else{
						if (pw.trap){
							// clear trap message
							LCD_Fill(5,242,45,260,BACKGROUND);
						}
						reset_wall_mode();
						// show possible shift
						view_possible_shifts(5);
					}
					break;
				default:
								
					break;
			}
		}
		else {	/* button released */
			down_1=0;			
			NVIC_EnableIRQ(EINT1_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 22);     /* External interrupt 1 pin selection */
		}
		}
		}
		
		// button EINT0
		if(down_0!=0){  
			down_0 ++;  
		if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){

			switch(down_0){
			case 2:
				game_mode = 1;
				break;
			default:
				break;
		}
	}
	else {	/* button released */
		down_0=0;			
		NVIC_EnableIRQ(EINT0_IRQn);							 /* enable Button interrupts			*/
		LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */
	}
	}
	
	// button EINT2
	if(down_2!=0){  
			down_2 ++;  
		if((LPC_GPIO2->FIOPIN & (1<<12)) == 0){
			switch(down_2){
			case 2:
				
				if (q.move_mode){
					restore_walls();
					pw.orientation += 1;
					pw.orientation %= 2;
					pw.valid = wall_check();
				}
				
				break;
			default:			
			
				break;
		}
	}
	else {	/* button released */
		down_2=0;			
		NVIC_EnableIRQ(EINT2_IRQn); 							 /* enable Button interrupts			*/
		LPC_PINCON->PINSEL4    |= (1 << 24);     /* External interrupt 2 pin selection */
	}
	}
		
	reset_RIT();
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
