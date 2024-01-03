/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           funct_joystick.h
** Last modified Date:  2018-12-30
** Last Version:        V1.00
** Descriptions:        High level joystick management functions
** Correlated files:    lib_joystick.c, funct_joystick.c
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "lpc17xx.h"
#include "joystick.h"
#include "../GLCD/GLCD.h"

/* access to move */
extern unsigned int move;

/* access to structures of the game */
extern Quoridor q;
extern PossibleShifts ps;

void joystick_player(char choice) {
	if(ps.up == (choice==0) || ps.down == (choice==1) || ps.left == (choice==2) || ps.right == (choice==3)){
		LCD_Possible_Shifts(choice,ps,q.players_pos[q.turn_index][0],q.players_pos[q.turn_index][1],q.players_pos[(q.turn_index+1)%2][0],q.players_pos[(q.turn_index+1)%2][1],2);
	}
}

void joystick_confirm(char choice){
	move = 0;
 	move |= (q.turn_index << 24);
	q.move_confirmed = 1;
	if (ps.up && choice==0){

		move |= q.players_pos[q.turn_index][0];
		move |= ((q.players_pos[q.turn_index][1]-1) << 8);
	}
	else if (ps.down && choice==1){
	
		move |= q.players_pos[q.turn_index][0];
		move |= ((q.players_pos[q.turn_index][1]+1) << 8);
	}
	else if (ps.left && choice==2){
	
		move |= q.players_pos[q.turn_index][0]-1;
		move |= ((q.players_pos[q.turn_index][1]) << 8);
	}
	else if (ps.right && choice==3){
		
		move |= q.players_pos[q.turn_index][0]+1;
		move |= ((q.players_pos[q.turn_index][1]) << 8);
	}
	else{
		// invalid move
		move = 0x010000;
		q.move_confirmed = 0;
	}
	if (q.remaining_walls[q.turn_index] == 0){
		LCD_Fill(5,242,310,260,BACKGROUND);
	}
}

void joystick_confirm_wall(char x,char y,char orientation){
	q.move_confirmed = 1;
	move = 0;
	move |= (q.turn_index << 24);
	move |= x;
	move |= (y << 8);
	move |= (1<<20);
	move |= (orientation << 16);
	q.remaining_walls[q.turn_index] -= 1;
}
