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
#include "../game.h"
#include "../Joystick/joystick.h"
#include "../timer/timer.h"


/* access at game structures */
extern Quoridor q;
extern PossibleWall pw;
extern PossibleShifts ps;
extern Can c;

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

volatile char J_upLeft;
volatile char J_upRight;
volatile char J_downLeft;
volatile char J_downRight;


void RIT_IRQHandler (void)
{		
		// init static variable for menu buttons 
		static char game_menu = 0;
		static char single_menu = 0;
		static char multi_menu = 0;
	
		static char wall_down = 0;
		static char wall_up = 0;
		static char wall_right = 0;
		static char wall_left = 0;
	
		static char j_select;
		
		
		/* check the mode of the game */
		switch(game_mode){
			
			//wait mode
			case 0:
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
				break;
			// game menu	
			case 1:
				
				// joystick up
				if ((LPC_GPIO1->FIOPIN & (1<<29)) == 0){
					// button two selected
					if (game_menu){
						select_button_mode(0);
						game_menu = 0;
					}
					break;
				}
			
				// joystick down
				if ((LPC_GPIO1->FIOPIN & (1<<26)) == 0){
					// shift mode
					if (game_menu == 0){
						select_button_mode(1);
						game_menu = 1;
					}
					break;
				}
			
				// joystick select
				if ((LPC_GPIO1->FIOPIN & (1<<25)) == 0){
 					j_select++;
					if (j_select == 1){
						if(game_menu){
							game_mode = 3;
						}
						else{
							game_mode = 2;
						}
						game_menu = 0;
					}
				}else{
					j_select = 0;
				}
				break;
				
			// single menu	
			case 2:
				
				// joystick up
				if ((LPC_GPIO1->FIOPIN & (1<<29)) == 0){
					// selected secon button
					if (single_menu){
						select_button_player(0);
						single_menu = 0;
					}
					break;
				}
			
				// joystick down
				if ((LPC_GPIO1->FIOPIN & (1<<26)) == 0){
					// selected first button
					if (single_menu == 0){
						select_button_player(1);
						single_menu = 1;
					}
					break;
				}
			
				// joystick select
				if ((LPC_GPIO1->FIOPIN & (1<<25)) == 0){
					j_select++;
					if(j_select == 1){
						if(single_menu){
							game_mode = 5;
						}
						else{
							game_mode = 4;
						}
						single_menu = 0;
					}
				}else{
					j_select = 0;
				}
				break;
			// multi menu	
			case 3:
				
				// joystick up
				if ((LPC_GPIO1->FIOPIN & (1<<29)) == 0){
					// selected second button
					if (multi_menu){
						select_button_player(0);
						multi_menu = 0;
					}
					break;
				}
			
				// joystick down
				if ((LPC_GPIO1->FIOPIN & (1<<26)) == 0){
					// selected first button
					if (multi_menu == 0){
						select_button_player(1);
						multi_menu = 1;
					}
					break;
				}
			
				// joystick select
				if ((LPC_GPIO1->FIOPIN & (1<<25)) == 0){
					if(c.received){
						j_select++;
						if(j_select == 1){
							if(multi_menu){
								game_mode = 7;
								
							}
							else{
								game_mode = 6;
								
							}
							multi_menu = 0;
						}
						break;
					}
				}else{
					j_select = 0;
				}
				// button EINT0
				if(down_0!=0){  
					down_0 ++;  
					if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){

						switch(down_0){
						case 2:
							if (c.received == 0){
								// send handshake
								start_handshake();
								// start timer 1
								enable_timer(1);
							}
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
				break;
				
				
				
		default:
			// check games mode
			if (game_mode==4 || (game_mode==5 && q.turn_index==0) || (game_mode==6 && c.opponent_ready && c.ready)){
				// joystick up left
				if (((LPC_GPIO1->FIOPIN & (1<<29)) == 0) && ((LPC_GPIO1->FIOPIN & (1<<27)) == 0)){
					// shift mode
					if (q.move_mode == 0){
 						J_upLeft++;
						if (J_upLeft==1){
							joystick_player(4);
							J_left = 0;
							J_right = 0;
							J_down = 0;
							J_up = 0;
							J_upRight = 0;
							J_downLeft = 0;
							J_downRight = 0;
						}
					}
					break;
				}
				
				// joystick up right
				if (((LPC_GPIO1->FIOPIN & (1<<29)) == 0) && ((LPC_GPIO1->FIOPIN & (1<<28)) == 0)){
					// shift mode
					if (q.move_mode == 0){
						J_upRight++;
						if (J_upRight==1){
							joystick_player(5);
							J_left = 0;
							J_right = 0;
							J_down = 0;
							J_up = 0;
							J_upLeft = 0;
							J_downLeft = 0;
							J_downRight = 0;
						}
					}
					break;
				}
			
				// joystick down left
				if (((LPC_GPIO1->FIOPIN & (1<<26)) == 0) && ((LPC_GPIO1->FIOPIN & (1<<27)) == 0)){
					// shift mode
					if (q.move_mode == 0){
						J_downLeft++;
						if (J_downLeft==1){
							joystick_player(6);
							J_left = 0;
							J_right = 0;
							J_down = 0;
							J_up = 0;
							J_upLeft = 0;
							J_upRight = 0;
							J_downRight = 0;
						}
					}
					break;
				}
			
				// joystick down right
				if (((LPC_GPIO1->FIOPIN & (1<<26)) == 0) && ((LPC_GPIO1->FIOPIN & (1<<28)) == 0)){
					// shift mode
					if (q.move_mode == 0){
						J_downRight++;
						if (J_downRight==1){
							joystick_player(7);
							J_left = 0;
							J_right = 0;
							J_down = 0;
							J_up = 0;
							J_upLeft = 0;
							J_upRight = 0;
							J_downLeft = 0;
						}
					}
					break;
				}
				
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
							J_upLeft = 0;
							J_upRight = 0;
							J_downLeft = 0;
							J_downRight = 0;
						}
					}
					// wall mode
					else if(q.move_mode && pw.position[1] != 0){
						if (wall_up % 5 == 0){
							restore_walls();
							pw.position[1] -= 1;
							pw.valid = wall_check(1);
						}
						wall_up++;
					}
					break;
				}else{
					wall_up = 0;
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
							J_upLeft = 0;
							J_upRight = 0;
							J_downLeft = 0;
							J_downRight = 0;
						}
					}
					// wall mode
					else if(q.move_mode && pw.position[1] != 5){
						if (wall_down % 5 == 0){
							restore_walls();
							pw.position[1] += 1;
							pw.valid = wall_check(1);
						}
						wall_down++;
					}
					break;
				}else{
					wall_down = 0;
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
							J_upLeft = 0;
							J_upRight = 0;
							J_downLeft = 0;
							J_downRight = 0;
						}
					}
					// wall mode
					else if(q.move_mode && pw.position[0] != 0){
						if (wall_left % 5 == 0){
							restore_walls();
							pw.position[0] -= 1;
							pw.valid = wall_check(1);
						}
						wall_left++;
					}
					break;
				}else{
					wall_left = 0;
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
							J_upLeft = 0;
							J_upRight = 0;
							J_downLeft = 0;
							J_downRight = 0;
						}
					}
					// wall mode
					else if(q.move_mode && pw.position[0] != 5){
						if (wall_right % 5 == 0){
							restore_walls();
							pw.position[0] += 1;
							pw.valid = wall_check(1);
						}
						wall_right++;
					}
					break;
				}else{
					wall_right = 0;
				}
				
				// joystick select
				if ((LPC_GPIO1->FIOPIN & (1<<25)) == 0){
					j_select++;
					if(j_select==1){
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
							}else if(J_down){
								joystick_confirm(1);
							}else if(J_left){
								joystick_confirm(2);
							}else if(J_right){
								joystick_confirm(3);
							}else if(J_upLeft){
								joystick_confirm(4);
							}else if(J_upRight){
								joystick_confirm(5);
							}else if(J_downLeft){
								joystick_confirm(6);
							}else if(J_downRight){
								joystick_confirm(7);
							}
						}
						J_left = 0;
						J_up = 0;
						J_down = 0;
						J_right = 0;
						J_upLeft = 0;
						J_upRight = 0;
						J_downLeft = 0;
						J_downRight = 0;
						break;
					}
					
				}else{
					j_select=0;
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
									pw.valid = wall_check(1);
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
								view_possible_shifts(9);
							}
							break;
						default:
										
							break;
						}
					}
					else {	
						down_1=0;			
						NVIC_EnableIRQ(EINT1_IRQn);							 
						LPC_PINCON->PINSEL4    |= (1 << 22);     
					}
					break;
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
								pw.valid = wall_check(1);
							}
							break;
						default:			
						
							break;
						}
					}
					else {	
						down_2=0;			
						NVIC_EnableIRQ(EINT2_IRQn); 							 
						LPC_PINCON->PINSEL4    |= (1 << 24);     
					}
					break;
				}
				
				break;
			}
		break;
	}
		
	reset_RIT();
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
