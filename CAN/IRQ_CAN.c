/*----------------------------------------------------------------------------
 * Name:    Can.c
 * Purpose: CAN interface for for LPC17xx with MCB1700
 * Note(s): see also http://www.port.de/engl/canprod/sv_req_form.html
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <lpc17xx.h>                  
#include "../CAN/CAN.h"                     
#include "../GLCD/GLCD.h"
#include "../game.h"
#include "../timer/timer.h"

extern uint8_t icr ; 						//icr and result must be global in order to work with both real and simulated landtiger.
extern uint32_t result;
extern CAN_msg CAN_TxMsg;    /* CAN message for sending */
extern CAN_msg CAN_RxMsg;    /* CAN message for receiving */                                


// access to game variables
extern unsigned int move;
extern Quoridor q;
extern Can c;
extern char game_mode;

/*----------------------------------------------------------------------------
  CAN interrupt handler
 *----------------------------------------------------------------------------*/
void CAN_IRQHandler (void)  {
	uint16_t key;

  /* check CAN controller 1 */
	icr = 0;
  icr = (LPC_CAN1->ICR | icr) & 0xFF;               /* clear interrupts */
	
  if (icr & (1 << 0)) {                          		/* CAN Controller #1 meassage is received */
		CAN_rdMsg (1, &CAN_RxMsg);	                		/* Read the message */
    LPC_CAN1->CMR = (1 << 2);                    		/* Release receive buffer */
		
		// get first data
		key = CAN_RxMsg.data[0];
		switch(key){
			// received init comunication
			case 0xff:
				if (game_mode < 4){
					q.opponent_id = 0;
					q.player_id = 1;
					c.ready = 0;
					c.opponent_ready = 0;
					if (game_mode==3){
						LCD_Fill(20,235,310,310,BACKGROUND);
						GUI_Text(30,240,(uint8_t *) "  Connection opened!", VALID,BACKGROUND);
						GUI_Text(28,260,(uint8_t *) "   You are Player 1!", TOKEN_2,BACKGROUND);
					}else{
						game_mode = 3;
					}
					c.received = 1;
					send_ok();
				}else{
					send_busy();
				}
				break;
			// received ok for the comunication
			case 0xfe:
				// disable timer
				reset_timer(1);
				disable_timer(1);
				c.received = 1;
				LCD_Fill(20,235,310,310,BACKGROUND);
				GUI_Text(30,240,(uint8_t *) "  Connection opened!", VALID,BACKGROUND);
				GUI_Text(28,260,(uint8_t *) "   You are Player 0!", TOKEN_1,BACKGROUND);
				break;
			
			// received busy for the comunication
			case 0xfd:
				// disable timer
				reset_timer(1);
				disable_timer(1);
				reset_connection();
				LCD_Fill(20,235,310,310,BACKGROUND);
				GUI_Text(30,240,(uint8_t *) " The opponent is busy!", NOT_VALID,BACKGROUND);
				GUI_Text(30,260,(uint8_t *) "   Press again INT0", NOT_VALID,BACKGROUND);
			  GUI_Text(30,280,(uint8_t *) " for a new connection!", NOT_VALID,BACKGROUND);
				break;
			
			// received opponent ready
			case 0xfc:
				c.opponent_ready = 1;
				if (!c.ready){
					LCD_Fill(20,235,310,310,BACKGROUND);
					GUI_Text(30,240, (uint8_t *) " The opponent is ready!",VALID,BACKGROUND);
				}
				break;
				
			default:
				move = 0;
				move |= CAN_RxMsg.data[3];;
				move = (move << 8) | CAN_RxMsg.data[2];
				move = (move << 8) | CAN_RxMsg.data[1];
				move = (move << 8) | CAN_RxMsg.data[0];
				q.move_confirmed = 1;
				break;
		}		
  }
	else
		if (icr & (1 << 1)) {                         /* CAN Controller #1 meassage is transmitted */
			// do nothing in this example
		}
}
