#include <stdio.h>
#include "GLCD/GLCD.h"
#include "timer/timer.h"
#include "RIT/RIT.h"
#include "CAN/CAN.h"

extern char game_mode;

/* DECLARATION OF STRUCTS */
volatile Quoridor q;
volatile PossibleWall pw;
volatile PossibleShifts ps;
volatile Can c;

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
char no_up_walls(char x, char y){
	return ! ((y == 0) || ((x>0) & (q.walls_center[x-1][y-1] == 1)) || ((x<6) && q.walls_center[x][y-1] == 1));
}
char no_down_walls(char x, char y){
	return ! ((y == 6) || ((x>0) & (q.walls_center[x-1][y] == 1)) || ((x<6) && q.walls_center[x][y] == 1));
}
char no_left_walls(char x, char y){
	return ! ((x == 0) || ((y<6) && q.walls_center[x-1][y] == 0) || ((y>0) && (q.walls_center[x-1][y-1] == 0)));
}
char no_right_walls(char x, char y){
	return ! ((x == 6) || ((y>0) && (q.walls_center[x][y-1] == 0)) || ((y<6) && q.walls_center[x][y] == 0));
}

char valid_up(char x, char y, char x_o, char y_o){
	return no_up_walls(x,y) &&
				 ! (x==x_o && (y-1)==y_o && ((y-2 < 0) || ((x>0) && q.walls_center[x-1][y-2] == 1) || (q.walls_center[x][y-2] == 1)));

}
char valid_down(char x, char y, char x_o, char y_o){
	return  no_down_walls(x,y) &&
					! (x==x_o && (y+1)==y_o && ((y+2 > 6) || ((x>0) && q.walls_center[x-1][y+1] == 1) || (q.walls_center[x][y+1] == 1)));
}
char valid_left(char x, char y, char x_o, char y_o){
	return  no_left_walls(x,y) &&
					! (y==y_o && (x-1)==x_o && ((x-2 < 0) || ((y>0) && q.walls_center[x-2][y-1] == 0) || (q.walls_center[x-2][y] == 0)));
	
}
char valid_right(char x, char y, char x_o, char y_o){
	return  no_right_walls(x,y) &&
					! (y==y_o && (x+1)==x_o && ((x+2 > 6) || ((y>0) && q.walls_center[x+1][y-1] == 0) || (q.walls_center[x+1][y] == 0)));
}

char is_up_left_condition(char x, char y, char x_o, char y_o){
	return (x==x_o && (y-1)==y_o && 
				 (y-2 < 0 || (q.walls_center[x][y-2] == 1 || (x>0 && q.walls_center[x-1][y-2] == 1))));
}
char is_left_up_condition(char x, char y, char x_o, char y_o){
	return (y==y_o && (x-1)==x_o && 
				 (x-2 < 0 || ((q.walls_center[x-2][y-1] == 0) ||	(x>0 && q.walls_center[x-2][y] == 0))));
}
char is_up_right_condition(char x, char y, char x_o, char y_o){
	return (x==x_o && (y-1)==y_o && 
				 (y-2 < 0 || q.walls_center[x][y-2] == 1 || (x>0 && q.walls_center[x-1][y-2] == 1)));
}
char is_right_up_condition(char x, char y, char x_o, char y_o){
	return (y==y_o && (x+1)==x_o && 
				 (x+1 > 5 || (y>0 && q.walls_center[x+1][y-1] == 0) ||	q.walls_center[x+1][y] == 0));
	
}
char is_down_left_condition(char x, char y, char x_o, char y_o){
	return (x==x_o && (y+1)==y_o && 
				 (y+1 > 5 || q.walls_center[x][y+1] == 1 ||	(x>0 && q.walls_center[x-1][y+1] == 1)));
	
}
char is_left_down_condition(char x, char y, char x_o, char y_o){
	return  y==y_o && (x-1)==x_o && 
				 (x-2<0 || ((y>0 && q.walls_center[x-2][y-1] == 0) ||	q.walls_center[x-2][y] == 0));
}
char is_down_right_condition(char x, char y, char x_o, char y_o){
	return (x==x_o && (y+1)==y_o && 
				 (y+1 > 5 || q.walls_center[x][y+1] == 1 ||	(x>0 && q.walls_center[x-1][y+1] == 1)));
	
}
char is_right_down_condition(char x, char y, char x_o, char y_o){
	return  y==y_o && (x+1)==x_o && 
				 (x+1 > 5 || (y<5 && q.walls_center[x+1][y] == 0) ||	q.walls_center[x+1][y-1] == 0);
	
}
char valid_up_left(char x, char y, char x_o, char y_o){
	return ((is_up_left_condition(x,y,x_o,y_o) &&
					no_up_walls(x,y) && no_left_walls(x,y-1)) ||
				 (is_left_up_condition(x,y,x_o,y_o) &&
					no_left_walls(x,y) && no_up_walls(x-1,y)));
}
char valid_up_right(char x, char y, char x_o, char y_o){
	return ((is_up_right_condition(x,y,x_o,y_o) &&
					no_up_walls(x,y) && no_right_walls(x,y-1)) ||
				 (is_right_up_condition(x,y,x_o,y_o) && 
					no_right_walls(x,y) && no_up_walls(x+1,y)));
}
char valid_down_left(char x, char y, char x_o, char y_o){
	return ((is_down_left_condition(x,y,x_o,y_o) &&
					no_down_walls(x,y) && no_left_walls(x,y+1)) ||
				 (is_left_down_condition(x,y,x_o,y_o) &&
				  no_left_walls(x,y) && no_down_walls(x-1,y)));
}
char valid_down_right(char x, char y, char x_o, char y_o){
	return ((is_down_right_condition(x,y,x_o,y_o) &&
					no_down_walls(x,y) && no_right_walls(x,y+1)) ||
				 (is_right_down_condition(x,y,x_o,y_o) &&
				 (no_right_walls(x,y) && no_down_walls(x+1,y))));
}

void update_possible_shifts(){
	// player
	char x = q.players_pos[q.turn_index][0];
	char y = q.players_pos[q.turn_index][1];
	// opponent
	char x_o = q.players_pos[(q.turn_index+1)%2][0];
	char y_o = q.players_pos[(q.turn_index+1)%2][1];
	
	// check top
	ps.up = valid_up(x,y,x_o,y_o);
	//check down
	ps.down = valid_down(x,y,x_o,y_o);
	// check left
	ps.left = valid_left(x,y,x_o,y_o);
	// check right
	ps.right = valid_right(x,y,x_o,y_o);
	// check up left
	ps.up_left = valid_up_left(x,y,x_o,y_o);
	// check up right
	ps.up_right = valid_up_right(x,y,x_o,y_o);
	// check down left
	ps.down_left = valid_down_left(x,y,x_o,y_o);
	// check down right
	ps.down_right = valid_down_right(x,y,x_o,y_o);
	/*
	if( ((x==x_o && (y+1)==y_o && (q.walls_center[x][y+1] == 1 ||	(x>0 && q.walls_center[x-1][y+1] == 1))) && 
		  (x>0 && q.walls_center[x-1][y] == 2 && q.walls_center[x-1][y+1] != 0 && q.walls_center[x][y] != 1 )) ||
			(y==y_o && (x-1)==x_o && (x-2<0 || ((q.walls_center[x-2][y+1] == 0) ||	(x>0 && q.walls_center[x-2][y] == 0))) &&
			 (x>0 && q.walls_center[x-1][y] == 2 && q.walls_center[x-2][y] != 1 && (q.walls_center[x-1][y-1] != 0))) ){
			ps.down_left = 1;
	}else{
		ps.down_left = 0;
	}
	if(((x==x_o && (y+1)==y_o && (q.walls_center[x][y+1] == 1 ||	(x>0 && q.walls_center[x-1][y+1] == 1))) && 
			 x<6 && q.walls_center[x][y] == 2 && q.walls_center[x][y+1] != 0  && q.walls_center[x-1][y] != 1 ) ||
			(y==y_o && (x+1)==x_o && ((q.walls_center[x][y+1] == 0) ||	(x>0 && q.walls_center[x][y] == 0)) &&
			(x>0 && q.walls_center[x][y] == 2 && (x+1 > 5 || (q.walls_center[x+1][y] != 1)) && q.walls_center[x][y-1] != 0 )) ){
		ps.down_right = 1;
	}else{
		ps.down_right = 0;
	}
	
	if( ((x==x_o && (y-1)==y_o && (y-2 < 0 || (q.walls_center[x][y-2] == 1 || (x>0 && q.walls_center[x-1][y-2] == 1)))) && 
		   (x>0 && q.walls_center[x-1][y-1] == 2 && (y-2 < 0 || q.walls_center[x-1][y-2] != 0 ) && q.walls_center[x][y-1] != 1)) ||
			 (y==y_o && (x-1)==x_o && (x-2 < 0 || ((q.walls_center[x-2][y-1] == 0) ||	(x>0 && q.walls_center[x-2][y] == 0))) &&
			 (x>0 && q.walls_center[x-1][y-1] == 2 && (x-2 < 0 || q.walls_center[x-2][y-1] != 1 ) && q.walls_center[x-1][y] != 0))  ){
			ps.up_left = 1;
	}else{
		ps.up_left = 0;
	}
		if( ((x==x_o && (y-1)==y_o && (y-2 < 0 || (q.walls_center[x][y-2] == 1 || (x>0 && q.walls_center[x-1][y-2] == 1)))) && 
				(x<6 && q.walls_center[x][y-1] == 2 && q.walls_center[x-1][y-1] != 1 && (y-2 < 0 || q.walls_center[x][y-2] != 0))) ||
				(y==y_o && (x+1)==x_o && ((q.walls_center[x+1][y-1] == 0) ||	(x>0 && q.walls_center[x+1][y] == 0)) &&
			  (x>0 && q.walls_center[x][y-1] == 2 && q.walls_center[x][y] != 0 && q.walls_center[x+1][y-1] != 1 )) ){
			ps.up_right = 1;
	}else{
		ps.up_right = 0;
	}
	*/
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
	LCD_Possible_Shifts(9,ps,q.players_pos[q.turn_index][0],q.players_pos[q.turn_index][1],q.players_pos[(q.turn_index+1)%2][0],q.players_pos[(q.turn_index+1)%2][1],3);
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
	
	if (flag_orientation == 1 & type_of_move == 0){
		// time over
		if (q.move_mode){
			reset_wall_mode();
			if (pw.trap){
				// clear trap message
				LCD_Fill(5,242,220,260,BACKGROUND);
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
				}
				else if (ps.down){
					// down
					if (new_x == q.players_pos[(player_id+1)%2][0] && new_y == q.players_pos[(player_id+1)%2][1]){
						new_y +=1;
					}
				}
			}
			else{
				if ((new_x < q.players_pos[player_id][0]) & ps.left){
					//left
					if (new_x == q.players_pos[(player_id+1)%2][0] && new_y == q.players_pos[(player_id+1)%2][1]){
						new_x -=1;
					}
				}
				else if (ps.right){
					//right
					if (new_x == q.players_pos[(player_id+1)%2][0] && new_y == q.players_pos[(player_id+1)%2][1]){
						new_x +=1;
					}
				}
			}
			
			// clear old poss
			clear_possible_shifts();
			LCD_Token(q.players_pos[player_id][0],q.players_pos[player_id][1],3);
			// new pos
			LCD_Token(new_x,new_y,player_id);
			q.players_pos[player_id][0] = new_x;
			q.players_pos[player_id][1] = new_y;
			
		}
		else{
			// moving a wall
			LCD_DrawWall(new_x,new_y,flag_orientation,3);
			q.walls_center[new_x][new_y] = flag_orientation;
			q.remaining_walls[q.turn_index] -= 1;
			
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
		  no_up_walls(x,y) && pw.visited_cells[x][y-1] == 0){
		pw.visited_cells[x][y-1] = 1;
		goal = search_goal_path(x,y-1,player_id) || goal;
	}
	//check down
	if (!goal &&
			no_down_walls(x,y) && pw.visited_cells[x][y+1] == 0){
		pw.visited_cells[x][y+1] = 1;
		goal = search_goal_path(x,y+1,player_id) || goal;
	}
	// check left
	if (!goal &&
			no_left_walls(x,y) && pw.visited_cells[x-1][y] == 0){
		pw.visited_cells[x-1][y] = 1;
		goal = search_goal_path(x-1,y,player_id) || goal;
	}
	// check right
	if (!goal &&
			no_right_walls(x,y) && pw.visited_cells[x+1][y] == 0){
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

char wall_check(char visualize){
	if (pw.trap && visualize){
		// clear trap message
		LCD_Fill(5,242,220,260,BACKGROUND);
	}
	// position already taken
	if (q.walls_center[pw.position[0]][pw.position[1]] != 2){
		if (visualize){
			LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,0);
		}
		return 0;
	}
	// check horizontal
	if (pw.orientation 
			&& ((((pw.position[0]-1) >= 0) && (q.walls_center[pw.position[0]-1][pw.position[1]] == pw.orientation)) 
			|| (((pw.position[0]+1) < 6) && (q.walls_center[pw.position[0]+1][pw.position[1]] == pw.orientation)))){
		if (visualize){
			LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,0);
		}
		return 0;
	}
	// check vertical
	if (pw.orientation == 0 &&
			((((pw.position[1]-1) >= 0) && (q.walls_center[pw.position[0]][pw.position[1]-1] == pw.orientation)) 
			|| (((pw.position[1]+1) < 6) && (q.walls_center[pw.position[0]][pw.position[1]+1] == pw.orientation)))){
		if (visualize){
			LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,0);
		}
		return 0;
	}
	// check trap
	pw.trap = 0;
	pw.trap = check_trap();
	if (pw.trap){
		if (visualize){
			LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,0);
			GUI_Text(5, 245 , (uint8_t *) "You cannot create a Trap!", NOT_VALID, BACKGROUND);
		}
		return 0;
	}
	if (visualize){
		LCD_DrawWall(pw.position[0],pw.position[1],pw.orientation,1);
	}
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
	LCD_Current_Player(q.turn_index);
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
	uint16_t TOKEN;
	if (id == 0){
		TOKEN = TOKEN_1;
	}else{
		TOKEN = TOKEN_2;
	}
	sprintf(pharse," Winner is Player %d!",id);
	LCD_Clear(BACKGROUND);
	GUI_Text(40, 100, (uint8_t *) pharse,TOKEN,BACKGROUND);
	GUI_Text(56, 140, (uint8_t *) "   Press INT0    ",TEXT,BACKGROUND);
	GUI_Text(56, 180, (uint8_t *) "to view the menu!",TEXT,BACKGROUND);
	 
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
	GUI_Text(30, 140, (uint8_t *) "      Press INT0     ",  TEXT,BACKGROUND);
	GUI_Text(32, 180, (uint8_t *) "   to view the menu! ",  TEXT,BACKGROUND);
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
	  disable_RIT();
		LCD_Current_Player(q.turn_index);
		// show number of remaining walls
		view_remaining_walls();
		// show possible shifts
		view_possible_shifts(9);
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
	
void play_single_human_human(){
	//initialiazation
	init_game();
	
	// loop on turn
	while(1){
		// compute possible shifts
		update_possible_shifts();
		// show the board
		show_board();
		// visualize time left
		LCD_Time(20);
		// start the timer
		enable_timer(0);
		// loop on move choice
		while(1){
			// time over or move confirmed
			if (q.time_left == 0 || q.move_confirmed){
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



// EXTRAPOINT 2

void view_game_menu(){
	LCD_Clear(BACKGROUND);
	GUI_Text(70, 60, (uint8_t *) " Select the",  TEXT, BACKGROUND);
	GUI_Text(70, 90, (uint8_t *) " GAME MODE! ",  TEXT,BACKGROUND);
	LCD_Button(60,140,180,170,"Single Boards",1);
	LCD_Button(60,190,180,220," Two Boards",0);
}


void view_single_menu(){
	LCD_Clear(BACKGROUND);
	GUI_Text(35, 60, (uint8_t *) " Single Board: select",  TEXT, BACKGROUND);
	GUI_Text(35, 90, (uint8_t *) " the opposite player! ",  TEXT,BACKGROUND);
	LCD_Button(60,140,180,170,"    Human",1);
	LCD_Button(60,190,180,220,"     NPC ",0);
}


void view_multi_menu(){
	LCD_Clear(BACKGROUND);
	GUI_Text(40, 60, (uint8_t *) " Two Boards: select",  TEXT, BACKGROUND);
	GUI_Text(40, 90, (uint8_t *) "    your player! ",  TEXT,BACKGROUND);
	LCD_Button(60,140,180,170,"    Human",1);
	LCD_Button(60,190,180,220,"     NPC ",0);
}



void start_app(){	
	while(1){
		switch(game_mode){
			case 1:
				view_game_menu();
				//start menu
				while(game_mode == 1){
					// wait a decision in the menu
				}
				break;
			// single menu
			case 2:
				view_single_menu();
				while(game_mode == 2){
					// wait a decision in the menu
				}
				break;
			//multi menu
			case 3:
				view_multi_menu();
				if (c.received){
					LCD_Fill(20,235,310,310,BACKGROUND);
					GUI_Text(30,240,(uint8_t *) "  Connection opened!", VALID,BACKGROUND);
					GUI_Text(30,260,(uint8_t *) "   You are Player 1!", TOKEN_2,BACKGROUND);
				}else{
					LCD_Fill(20,235,310,310,BACKGROUND);
					GUI_Text(30,240,(uint8_t *) "    Press INT0 to", TEXT,BACKGROUND);
					GUI_Text(30,260,(uint8_t *) "  open a connection!", TEXT,BACKGROUND);
				}
				while(game_mode == 3){
					// wait a decision in the menu
				}
				break;
			//play single - human vs human
			case 4:
				play_single_human_human();
				// after the game
				game_mode = 0;
				return;
			//play single - human vs npc
			case 5:
				play_single_human_ai();
				game_mode = 0;
				return;
			case 6:
				send_ready();
				if (c.opponent_ready == 0){
					LCD_Fill(20,235,310,310,BACKGROUND);
					GUI_Text(20,240,(uint8_t *) "You selected the Human mode", VALID,BACKGROUND);
					GUI_Text(30,260,(uint8_t *) "Waiting the opponent..", VALID,BACKGROUND);
					
				}
			  while(!c.opponent_ready){
					// wait the opponent
				}
				play_multi();
				game_mode = 0;
			  return;
			case 7:
				send_ready();
				if (c.opponent_ready == 0){
					LCD_Fill(20,235,310,310,BACKGROUND);
					GUI_Text(20,240,(uint8_t *) "You selected the NPC mode", VALID,BACKGROUND);
					GUI_Text(30,260,(uint8_t *) " Waiting opponent...", VALID,BACKGROUND);
				}
			  while(!c.opponent_ready){
					// wait the opponent
				}
				play_multi_with_ai();
				game_mode = 0;
				return;
			default:
				break;
		}
	}
}

/* AI implementation */

OptimalShift os;
BestWall bw;

char get_distance_to_goal(char x,char y,char player_id){
	
	char max_depth;
	char distance;
	for (max_depth = 1; max_depth < 43; max_depth++){
		clear_visited_cells();
		pw.visited_cells[x][y] = 1;
		distance = limited_depth_first_search(x,y,player_id,0,max_depth);
		if (distance != 0){
			return distance;
		}
	}
	return 0;
}



char limited_depth_first_search(char x,char y,char player_id,char depth,char max_depth){
	
	char goal = 0;
	
	if ((y == 6 && player_id) || (y == 0 && (player_id == 0))){
		return depth;
	}
	
	if (depth==max_depth){
		return 0;
	}
	
	// check top
	if (!goal &&
		  no_up_walls(x,y) && pw.visited_cells[x][y-1] == 0){
		pw.visited_cells[x][y-1] = 1;
		goal = limited_depth_first_search(x,y-1,player_id,depth+1,max_depth) | goal;
		pw.visited_cells[x][y-1] = 0;
	}
	//check down
	if (!goal &&
			no_down_walls(x,y) && pw.visited_cells[x][y+1] == 0){
		pw.visited_cells[x][y+1] = 1;
		goal = limited_depth_first_search(x,y+1,player_id,depth+1,max_depth) | goal;
		pw.visited_cells[x][y+1] = 0;
	}
	// check left
	if (!goal &&
			no_left_walls(x,y) && pw.visited_cells[x-1][y] == 0){
		pw.visited_cells[x-1][y] = 1;
		goal = limited_depth_first_search(x-1,y,player_id,depth+1,max_depth) | goal;
		pw.visited_cells[x-1][y] = 0;
	}
	// check right
	if (!goal &&
			no_right_walls(x,y) && pw.visited_cells[x+1][y] == 0){
		pw.visited_cells[x+1][y] = 1;
		goal = limited_depth_first_search(x+1,y,player_id,depth+1,max_depth) | goal;
		pw.visited_cells[x+1][y] = 0;
	}
	
	return goal;
}



void ai_move(){
	int i;
	int j;
	int o;
	char valid;
	char apply = 0;

	
	char opponent_distance;
	char npc_distance;
	
	// compute the distance to goal of the Opponent
	opponent_distance = get_distance_to_goal(q.players_pos[q.opponent_id][0],q.players_pos[q.opponent_id][1],q.opponent_id);
	// compute the distance to goal of the Ai
	npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0],q.players_pos[q.player_id][1],q.player_id);
	// compute the current difference distance to goal
	bw.best_dif_distance = npc_distance-opponent_distance;
	
	// check if the ai is more far away to goal than his opponent
	if (bw.best_dif_distance>=0 && q.remaining_walls[q.player_id]>0){
		
		for (i = 0; i<6; ++i){
			for (j = 0; j<6; ++j){
				for (o = 0; o<2; ++o){
					pw.position[0] = i;
					pw.position[1] = j;
					pw.orientation = o;
					valid = wall_check(0);
					if (valid){
						// simulate wall
						q.walls_center[i][j] = o;
						// get new distance to goal for the opponenet
						opponent_distance = get_distance_to_goal(q.players_pos[q.opponent_id][0],q.players_pos[q.opponent_id][1],q.opponent_id);
						npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0],q.players_pos[q.player_id][1],q.player_id);
						// restore wall
						q.walls_center[i][j] = 2;
						// if the distance is higher update the best wall
						if ((npc_distance-opponent_distance)<bw.best_dif_distance){
							apply = 1;
							bw.best_dif_distance = npc_distance-opponent_distance;
							bw.position[0] = i;
							bw.position[1] = j;
							bw.orientation = o;
						}
					}
				}
			}
		}
		// apply wall
		if (apply){
			move = 0;
			move |= (q.turn_index << 24);
			move |=  bw.position[0];
			move |= (bw.position[1] << 8);
			move |= (1<<20);
			move |= (bw.orientation << 16);
			q.move_confirmed = 1;
		}
	}
	// apply shift if the npc is in advantage or cannot piece a good wall
	if(apply == 0){
		// compute the distance to goal of the Opponent
		opponent_distance = get_distance_to_goal(q.players_pos[q.opponent_id][0],q.players_pos[q.opponent_id][1],q.opponent_id);
		if (ps.up){
			// compute the distance to goal of the Ai
			if ((q.players_pos[q.player_id][0] == q.players_pos[q.opponent_id][0]) && q.players_pos[q.player_id][1]-1 == q.players_pos[q.opponent_id][1]){
				npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0],q.players_pos[q.player_id][1]-2,q.player_id);
				if (npc_distance-opponent_distance < bw.best_dif_distance){
					bw.best_dif_distance = npc_distance-opponent_distance;
					os.x = q.players_pos[q.player_id][0];
					os.y = q.players_pos[q.player_id][1]-2;
				}
			}else{
				npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0],q.players_pos[q.player_id][1]-1,q.player_id);
				if (npc_distance-opponent_distance < bw.best_dif_distance){
					bw.best_dif_distance = npc_distance-opponent_distance;
					os.x = q.players_pos[q.player_id][0];
					os.y = q.players_pos[q.player_id][1]-1;
				}
			}
			
		}
		if (ps.down){
			if ((q.players_pos[q.player_id][0] == q.players_pos[q.opponent_id][0]) && q.players_pos[q.player_id][1]+1 == q.players_pos[q.opponent_id][1]){
				// compute the distance to goal of the Ai
				npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0],q.players_pos[q.player_id][1]+2,q.player_id);
				if (npc_distance-opponent_distance < bw.best_dif_distance){
					bw.best_dif_distance = npc_distance-opponent_distance;
					os.x = q.players_pos[q.player_id][0];
					os.y = q.players_pos[q.player_id][1]+2;
				}

			}
			else{
				// compute the distance to goal of the Ai
				npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0],q.players_pos[q.player_id][1]+1,q.player_id);
			
				if (npc_distance-opponent_distance < bw.best_dif_distance){
					bw.best_dif_distance = npc_distance-opponent_distance;
					os.x = q.players_pos[q.player_id][0];
					os.y = q.players_pos[q.player_id][1]+1;
				
				}
			}
		}
		if (ps.left){
			if ((q.players_pos[q.player_id][1] == q.players_pos[q.opponent_id][1]) && q.players_pos[q.player_id][0]-1 == q.players_pos[q.opponent_id][1]){
				// compute the distance to goal of the Ai
				npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0]-2,q.players_pos[q.player_id][1],q.player_id);
				if (npc_distance-opponent_distance < bw.best_dif_distance){
					bw.best_dif_distance = npc_distance-opponent_distance;
					os.x = q.players_pos[q.player_id][0]-2;
					os.y = q.players_pos[q.player_id][1];
				}

			}else{
				// compute the distance to goal of the Ai
				npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0]-1,q.players_pos[q.player_id][1],q.player_id);
				if (npc_distance-opponent_distance < bw.best_dif_distance){
					bw.best_dif_distance = npc_distance-opponent_distance;
					os.x = q.players_pos[q.player_id][0]-1;
					os.y = q.players_pos[q.player_id][1];
				}
			}
		}
		if (ps.right){
			if ((q.players_pos[q.player_id][1] == q.players_pos[q.opponent_id][1]) && q.players_pos[q.player_id][0]+1 == q.players_pos[q.opponent_id][1]){
				// compute the distance to goal of the Ai
				npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0]+2,q.players_pos[q.player_id][1],q.player_id);
				if (npc_distance-opponent_distance < bw.best_dif_distance){
					bw.best_dif_distance = npc_distance-opponent_distance;
					os.x = q.players_pos[q.player_id][0]+2;
					os.y = q.players_pos[q.player_id][1];
				}

			}else{
				// compute the distance to goal of the Ai
				npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0]+1,q.players_pos[q.player_id][1],q.player_id);
				if (npc_distance-opponent_distance < bw.best_dif_distance){
					bw.best_dif_distance = npc_distance-opponent_distance;
					os.x = q.players_pos[q.player_id][0]+1;
					os.y = q.players_pos[q.player_id][1];
				}
			}
		}
		if (ps.up_left){
			// compute the distance to goal of the Ai
			npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0]-1,q.players_pos[q.player_id][1]-1,q.player_id);
			if (npc_distance-opponent_distance < bw.best_dif_distance){
				bw.best_dif_distance = npc_distance-opponent_distance;
				os.x = q.players_pos[q.player_id][0]-1;
				os.y = q.players_pos[q.player_id][1]-1;
				}
		}
		
		if (ps.up_right){
			// compute the distance to goal of the Ai
			npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0]+1,q.players_pos[q.player_id][1]-1,q.player_id);
			if (npc_distance-opponent_distance < bw.best_dif_distance){
				bw.best_dif_distance = npc_distance-opponent_distance;
				os.x = q.players_pos[q.player_id][0]+1;
				os.y = q.players_pos[q.player_id][1]-1;
			}
		}
		if (ps.down_left){
			// compute the distance to goal of the Ai
			npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0]-1,q.players_pos[q.player_id][1]+1,q.player_id);
			if (npc_distance-opponent_distance < bw.best_dif_distance){
				bw.best_dif_distance = npc_distance-opponent_distance;
				os.x = q.players_pos[q.player_id][0]-1;
				os.y = q.players_pos[q.player_id][1]+1;
			}
		}
		
		if (ps.down_right){
			// compute the distance to goal of the Ai
			npc_distance = get_distance_to_goal(q.players_pos[q.player_id][0]+1,q.players_pos[q.player_id][1]+1,q.player_id);
			if (npc_distance-opponent_distance < bw.best_dif_distance){
				bw.best_dif_distance = npc_distance-opponent_distance;
				os.x = q.players_pos[q.player_id][0]+1;
				os.y = q.players_pos[q.player_id][1]+1;
			}
		}
		move = 0;
		move |= (q.turn_index << 24);
		move |= os.x;
		move |= os.y << 8;
		q.move_confirmed = 1;	
	}
	//reset possible wall
	reset_wall_mode();
}



void play_single_human_ai(){
		//initialiazation
		init_game();
		// set the ai id
		q.player_id = 1;
		// my id is the opponent id for the ai
		q.opponent_id = 0;
		// loop on turn
		while(1){
			// compute possible shifts
			update_possible_shifts();
			if (q.turn_index == 0){
				// show the board
				show_board();
				// visualize time left
				LCD_Time(20);
			}
			else{
				// show number of remaining walls
				view_remaining_walls();
			}
			// start the timer
			enable_timer(0);
			// loop on move choice
			while(1){
				// ai choose the move
				if (q.turn_index == 1){
					ai_move();
					delay_ms(MS);
				}
				
				// time over or move confirmed
				if (q.time_left == 0 || q.move_confirmed){
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
void play_multi(){
		//initialiazation
		init_game();
		// loop on turn
		while(1){
			// compute possible shifts
			update_possible_shifts();
			if (q.turn_index == q.player_id){
				reset_timer(0);
				enable_timer(0);
				// show possible shifts
				view_possible_shifts(9);
				reset_RIT();
				enable_RIT();
			}else{
				reset_timer(0);
				reset_RIT();
				disable_RIT();
			}
			// show number of remaining walls
			view_remaining_walls();
			// show turn player
			LCD_Current_Player(q.turn_index);
			// visualize time left
			LCD_Time(20);
			// loop on move choice
			while(1){
				// time over or move confirmed
				if (q.time_left == 0 || q.move_confirmed){
					// reset timer
					disable_timer(0);
					// disable rit during lcd operation
					disable_RIT();
					
					if (q.turn_index == q.player_id){
						//send move
						send_move();
					}
					
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
					break;
				}
			}
		}
}

void play_multi_with_ai(){
		//initialiazation
		init_game();
		// loop on turn
		while(1){
			// compute possible shifts
			update_possible_shifts();
			reset_timer(0);
			enable_timer(0);
			disable_RIT();
			disable_timer(0);
			view_remaining_walls();
			LCD_Time(20);
			// loop on move choice
			while(1){
				
				if (q.turn_index == q.player_id){
					ai_move();
					delay_ms(MS);
				}
				// time over or move confirmed
				if (q.time_left == 0 || q.move_confirmed){
					// reset timer
					disable_timer(0);
					// disable rit during lcd operation
					disable_RIT();
					
					if (q.turn_index == q.player_id){
						//send move
						send_move();
					}
					
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
					break;
				}
			}
		}
}
