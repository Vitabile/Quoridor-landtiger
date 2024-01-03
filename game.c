#include <stdio.h>
#include "GLCD/GLCD.h"
#include "timer/timer.h"
#include "RIT/RIT.h"

extern char game_mode;

/* DECLARATION OF STRUCTS */
volatile Quoridor q;
volatile PossibleWall pw;
volatile PossibleShifts ps;

/* DECLARATION OF MOVE */
volatile unsigned int move;

/******************************************************************************
** Function name:		clear_visited_cells
**
** Descriptions:		Initialization of visited cells 
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void clear_visited_cells(){
	int i;
	int j;
	for (i = 0; i < 7; i++) {
		for (j = 0; j < 7; j++) {
			pw.visited_cells[i][j] = 0;
		}  
  }
}

/******************************************************************************
** Function name:		init_game
**
** Descriptions:		Initialization of the game
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void init_game(){
	int i;
	int j;
	
	// init move
	move = 0x010000; // init as skip turn
	
	// init possibleWall
	pw.orientation = 1;
	pw.position[0] = 3;
	pw.position[1] = 2;
	pw.valid = 0;
	clear_visited_cells();
	
	// init possibleShift
	ps.up = 0;
	ps.down = 0;
	ps.left = 0;
	ps.right = 0;
	
	// init quoridor
	q.move_mode = 0;
	q.move_confirmed = 0;
	q.win = 0;

	q.time_left = 20;
	q.remaining_walls[0] = 8;
	q.remaining_walls[1] = 8;

	q.turn_index = 0;
	
	q.players_pos[0][0] = 3;
	q.players_pos[0][1] = 6;
	q.players_pos[1][0] = 3;
	q.players_pos[1][1] = 0;

	// init walls
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 6; j++) {
			q.walls_center[i][j] = 2;
		}  
  }
	// show board
	LCD_Init_Board(q.players_pos);
}

/******************************************************************************
** Function name:		update_possible_shifts
**
** Descriptions:		Update possible shifts considering
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void update_possible_shifts(){
	// player
	char x = q.players_pos[q.turn_index][0];
	char y = q.players_pos[q.turn_index][1];
	// opponent
	char x_o = q.players_pos[(q.turn_index+1)%2][0];
	char y_o = q.players_pos[(q.turn_index+1)%2][1];
	
	// check top
	if ((y == 0) || 
			(x==x_o && (y-1)==y_o && ((y-2 < 0) || ((x>0) && q.walls_center[x-1][y-2] == 1) || (q.walls_center[x][y-2] == 1))) ||
			((x>0) & (q.walls_center[x-1][y-1] == 1)) || 
			((x<6) && q.walls_center[x][y-1] == 1)){
		ps.up = 0;
	}
	else{
		ps.up = 1;
	}
	//check down
	if ((y == 6) || 
			(x==x_o && (y+1)==y_o && ((y+2 > 6) || ((x>0) && q.walls_center[x-1][y+1] == 1) || (q.walls_center[x][y+1] == 1))) || 
			((x>0) & (q.walls_center[x-1][y] == 1)) || ((x<6) && q.walls_center[x][y] == 1)){
		ps.down = 0;
	}
	else{
		ps.down = 1;
	}
	// check left
	if ((x == 0) || 
		  (y==y_o && (x-1)==x_o && ((x-2 < 0) || ((y>0) && q.walls_center[x-2][y-1] == 0) || (q.walls_center[x-2][y] == 0))) || 
	    ((y<6) && q.walls_center[x-1][y] == 0) || ((y>0) & (q.walls_center[x-1][y-1] == 0))){
		ps.left = 0;
	}
	else{
		ps.left = 1;
	}
	// check right
	if ((x == 6) || 
		  (y==y_o && (x+1)==x_o && ((x+2 < 0) || ((y>0) && q.walls_center[x+1][y-1] == 0) || (q.walls_center[x+1][y] == 0))) || 
			((y>0) && (q.walls_center[x][y-1] == 0)) || 
			((y<6) && q.walls_center[x][y] == 0)){
		ps.right = 0;
	}
	else{
		ps.right = 1;
	}
}

/******************************************************************************
** Function name:		clear_possible_shifts
**
** Descriptions:		Delete possible shifts from lcd
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void clear_possible_shifts(){
	LCD_Possible_Shifts(5,ps,q.players_pos[q.turn_index][0],q.players_pos[q.turn_index][1],q.players_pos[(q.turn_index+1)%2][0],q.players_pos[(q.turn_index+1)%2][1],3);
}

/******************************************************************************
** Function name:		reset_wall_mode
**
** Descriptions:		Delete possible wall and re-init it
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void reset_wall_mode(){
	if (pw.valid){
		LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,2);
	}else{
		restore_walls();
	}
	q.move_mode = 0;
	pw.orientation = 1;
	pw.position[0] = 3;
	pw.position[1] = 2;
	pw.valid = 0;
}

/******************************************************************************
** Function name:		apply_move
**
** Descriptions:		Apply the shift/wall move
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
	
void apply_move(){
 	char player_id = (move >> 24);
	char new_x = move & 0xff;
	char new_y = (move >> 8) & 0xff;
	char flag_orientation = (move >> 16) & 0xf;
	char type_of_move = (move >> 20) & 0xf;
	
	char valid = 0;
	
	if (flag_orientation == 1 & type_of_move == 0){
		// time over
		if (q.move_mode){
			reset_wall_mode();
			if (pw.trap){
				// clear trap message
				LCD_Fill(5,242,45,260,BACKGROUND);
			}
		}
		else{
			clear_possible_shifts();
		}
	}
	else{
		if (type_of_move == 0){ 
			//moving player
			if (new_x == q.players_pos[player_id][0]){
				if ((new_y < q.players_pos[player_id][1]) & ps.up){
					// top
					if (new_x == q.players_pos[(player_id+1)%2][0] && new_y == q.players_pos[(player_id+1)%2][1]){
						new_y -=1;
					}
					valid = 1;
				}
				else if (ps.down){
					// down
					if (new_x == q.players_pos[(player_id+1)%2][0] && new_y == q.players_pos[(player_id+1)%2][1]){
						new_y +=1;
					}
					valid = 1;
				}
			}
			else{
				if ((new_x < q.players_pos[player_id][0]) & ps.left){
					//left
					if (new_x == q.players_pos[(player_id+1)%2][0] && new_y == q.players_pos[(player_id+1)%2][1]){
						new_x -=1;
					}
					valid = 1;
				}
				else if (ps.right){
					//right
					if (new_x == q.players_pos[(player_id+1)%2][0] && new_y == q.players_pos[(player_id+1)%2][1]){
						new_x +=1;
					}
					valid = 1;
				}
			}
			if(valid){
				// clear old poss
				clear_possible_shifts();
				LCD_Token(q.players_pos[player_id][0],q.players_pos[player_id][1],3);
				// new pos
				LCD_Token(new_x,new_y,player_id);
				q.players_pos[player_id][0] = new_x;
				q.players_pos[player_id][1] = new_y;
			}
		}
		else{
			// moving a wall
			LCD_DrawWall(new_x,new_y,flag_orientation,3);
			q.walls_center[new_x][new_y] = flag_orientation;
			
		}
	}
	q.turn_index += 1;
	q.turn_index %= 2;
}

/******************************************************************************
** Function name:		check_winner
**
** Descriptions:		Check if one of the players has won
**
** parameters:			None
** Returned value:		1 for winner, 0 else
**
******************************************************************************/

char check_winner(){
	if (q.players_pos[0][1] == 0 || q.players_pos[1][1] == 6){
		//LCD_Win(q.turn_index);
		return 1;
	}
	return 0;
}

/******************************************************************************
** Function name:		search_goal_path
**
** Descriptions:		Recursive function that search a possible goal path for a player
**
** parameters:			None
** Returned value:	None
**
******************************************************************************/

 char search_goal_path(char x,char y,char player_id){
	
	char goal = 0;
	
	if ((y == 6 && player_id) || (y == 0 && (player_id == 0))){
		return 1;
	}
	
	// check top
	if (!goal &&
		  !((y == 0) || 
			((x>0) && (q.walls_center[x-1][y-1] == 1)) || 
			((x<6) && q.walls_center[x][y-1] == 1) ||
			(pw.visited_cells[x][y-1] == 1))){
		pw.visited_cells[x][y-1] = 1;
		goal = search_goal_path(x,y-1,player_id) || goal;
	}
	//check down
	if (!goal &&
			!((y == 6) || 
				((x>0) && (q.walls_center[x-1][y] == 1)) || 
				((x<6) && q.walls_center[x][y] == 1) ||
				(pw.visited_cells[x][y+1] == 1))){
		pw.visited_cells[x][y+1] = 1;
		goal = search_goal_path(x,y+1,player_id) || goal;
	}
	// check left
	if (!goal &&
			!((x == 0) || 
				((y<6) && q.walls_center[x-1][y] == 0) || 
				((y>0) && (q.walls_center[x-1][y-1] == 0)) ||
				(pw.visited_cells[x-1][y] == 1))){
		pw.visited_cells[x-1][y] = 1;
		goal = search_goal_path(x-1,y,player_id) || goal;
	}
	// check right
	if (!goal &&
			!((x == 6) || 
			 ((y>0) && (q.walls_center[x][y-1] == 0)) || 
			 ((y<6) && q.walls_center[x][y] == 0) ||
			 (pw.visited_cells[x+1][y] == 1))){
		pw.visited_cells[x+1][y] = 1;
		goal = search_goal_path(x+1,y,player_id) || goal;
	}
	
	return goal;
}

/******************************************************************************
** Function name:		check_trap
**
** Descriptions:		Check if one of the two player is trapped
**
** parameters:			None
** Returned value:	1 if trapped, 0 otherwise
**
******************************************************************************/

char check_trap(){
	char goal_1;
	char goal_2;
	
	// player
	char x = q.players_pos[q.turn_index][0];
	char y = q.players_pos[q.turn_index][1];
	// opponent
	char x_o = q.players_pos[(q.turn_index+1)%2][0];
	char y_o = q.players_pos[(q.turn_index+1)%2][1];
	
	//simulate wall
	q.walls_center[pw.position[0]][pw.position[1]] = pw.orientation; //simulate the wall
	
	clear_visited_cells();
	pw.visited_cells[x][y] = 1;
	goal_1 = search_goal_path(x,y,q.turn_index);
	clear_visited_cells();
	if (goal_1 == 0){
		q.walls_center[pw.position[0]][pw.position[1]] = 2; //delete simulation
		return 1;
	}
	pw.visited_cells[x_o][y_o] = 1;
	goal_2 = search_goal_path(x_o,y_o,(q.turn_index+1)%2);
	q.walls_center[pw.position[0]][pw.position[1]] = 2; //delete simulation
	
	return !goal_2;
	
}

/******************************************************************************
** Function name:		wall_check
**
** Descriptions:		Check if the possible wall it's valid or not
**
** parameters:			None
** Returned value:		1 for valid, 0 otherwise
**
******************************************************************************/

char wall_check(){
	if (pw.trap){
		// clear trap message
		LCD_Fill(5,242,45,260,BACKGROUND);
	}
	// position already taken
	if (q.walls_center[pw.position[0]][pw.position[1]] != 2){
		LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,0);
		return 0;
	}
	// check horizontal
	if (pw.orientation 
			&& ((((pw.position[0]-1) > 0) && (q.walls_center[pw.position[0]-1][pw.position[1]] == pw.orientation)) 
			|| (((pw.position[0]+1) < 6) && (q.walls_center[pw.position[0]+1][pw.position[1]] == pw.orientation)))){
		LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,0);
		return 0;
	}
	// check vertical
	if (pw.orientation == 0 &&
			((((pw.position[1]-1) > 0) && (q.walls_center[pw.position[0]][pw.position[1]-1] == pw.orientation)) 
			|| (((pw.position[1]+1) < 6) && (q.walls_center[pw.position[0]][pw.position[1]+1] == pw.orientation)))){
		LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,0);
		return 0;
	}
	// check trap
	pw.trap = 0;
	pw.trap = check_trap();
	if (pw.trap){
		LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,0);
		GUI_Text(5, 245 , (uint8_t *) "Trap!", NOT_VALID, BACKGROUND);
		return 0;
	}
	LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,1);
	return 1;	
}

/******************************************************************************
** Function name:		restore_walls
**
** Descriptions:		Restore the original walls when a possible wall
**  								is moved in the board
**
** parameters:			None
** Returned value:	None
**
******************************************************************************/

void restore_walls(){
	// clear the actual position
	LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,2);
	if (pw.valid){
		// exit if was a valid position (no overlap)
		return;
	}
	// restore a wall in the same position
	if (q.walls_center[pw.position[0]][pw.position[1]] == pw.orientation){
		LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,3);
	}
	if(q.walls_center[pw.position[0]][pw.position[1]] == ((pw.orientation+1)%2)){	
	LCD_DrawWall(pw.position[0],pw.position[1],((pw.orientation+1)%2),3);
	}
	// restore confinant walls (horizontal)
	if (pw.orientation){
		if (pw.position[0] > 0 && q.walls_center[pw.position[0]-1][pw.position[1]] == pw.orientation){
			LCD_DrawWall(pw.position[0]-1,pw.position[1],pw.orientation,3);
		}
		if (pw.position[0] < 5 && q.walls_center[pw.position[0]+1][pw.position[1]] == pw.orientation){
			LCD_DrawWall(pw.position[0]+1,pw.position[1],pw.orientation,3);
		}
	}
	// restore confinant walls (vertical)
	else if(pw.orientation == 0){
		if (pw.position[1] > 0 && q.walls_center[pw.position[0]][pw.position[1]-1] == pw.orientation){
			LCD_DrawWall(pw.position[0],pw.position[1]-1,pw.orientation,3);
		}
		if (pw.position[1] < 5 && q.walls_center[pw.position[0]][pw.position[1]+1] == pw.orientation){
			LCD_DrawWall(pw.position[0],pw.position[1]+1,pw.orientation,3);
		}
	}
}

/******************************************************************************
** Function name:		view_possible_shifts
**
** Descriptions:		Shows in the lcd the possible position for a player move
**
** parameters:			None
** Returned value:	None
**
******************************************************************************/

void view_possible_shifts(char choice){
	LCD_Possible_Shifts(choice,ps,q.players_pos[q.turn_index][0],q.players_pos[q.turn_index][1],q.players_pos[(q.turn_index+1)%2][0],q.players_pos[(q.turn_index+1)%2][1],2);
}

/******************************************************************************
** Function name:		view_remaining_walls
**
** Descriptions:		Shows in the lcd the possible number of remaining walls
**
** parameters:			None
** Returned value:	None
**
******************************************************************************/

void view_remaining_walls(){
	LCD_Num_Walls(q.remaining_walls[0],q.remaining_walls[1]);
}

/******************************************************************************
** Function name:		view_winner_screen
**
** Descriptions:		Shows in the lcd the winning screen
**
** parameters:			None
** Returned value:	None
**
******************************************************************************/

void view_winner_screen(){
	char id = (q.turn_index+1)%2;
	char pharse[30] = "";
	
	sprintf(pharse,"Winner is Player %d!",id);
	LCD_Clear(BACKGROUND);
	GUI_Text(40, 100, (uint8_t *) pharse,TEXT,BACKGROUND);
	GUI_Text(72, 140, (uint8_t *) "Press INT0",TEXT,BACKGROUND);
	GUI_Text(56, 180, (uint8_t *) "to play again!",TEXT,BACKGROUND);
	 
}

/******************************************************************************
** Function name:		view_start_screen
**
** Descriptions:		Shows in the lcd the start screen
**
** parameters:			None
** Returned value:	None
**
******************************************************************************/

void view_start_screen(){
	LCD_Clear(BACKGROUND);
	GUI_Text(30, 100, (uint8_t *) " Welcome to Quoridor!",  TEXT, BACKGROUND);
	GUI_Text(53, 140, (uint8_t *) " Press INT0 to ",  TEXT,BACKGROUND);
	GUI_Text(53, 180, (uint8_t *) " start a Game! ",  TEXT,BACKGROUND);
}

/******************************************************************************
** Function name:		show_board
**
** Descriptions:		Shows in the lcd the new possible shifts and number of walls
**
** parameters:			None
** Returned value:	None
**
******************************************************************************/

void show_board(){
		//before animation disable rit (in this way we cannot broke the lcd with interrupt)
		disable_RIT(); 
	
		// show number of remaining walls
		view_remaining_walls();
		// show possible shifts
		view_possible_shifts(5);
	
	  // after animation enable rit
		enable_RIT(); 
}


/******************************************************************************
** Function name:		play
**
** Descriptions:		Play a match until a winner
**
** parameters:			None
** Returned value:	None
**
******************************************************************************/
	
void play(){
	//initialiazation
	init_game();
	
	// loop on turn
	while(1){
		// compute possible shifts
		update_possible_shifts();
		// show the board
		show_board();
		// start the timer
		enable_timer(0);
	
		// loop on move choice
		while(1){
			// time over or move confirmed
			if (q.time_left == 255 || q.move_confirmed){
				// reset timer
				reset_timer(0);
				
				// disable rit during lcd operation
				disable_RIT();
				// apply the move
				apply_move();
				
				//check winner
				q.win = check_winner();
				if (q.win){
					view_winner_screen();
					game_mode = 0;
					enable_RIT();
					return;
				}
				
				// reset default move on skip turn
				move = 0x010000;
				
				// reset timer count and flag of confirmed move
				q.time_left = 20;
				q.move_confirmed = 0;
				enable_RIT();
				break;
			}
		}
	}
}

