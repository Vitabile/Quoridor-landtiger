#ifndef GAME_H
#define GAME_H

/* STRUCTS FOR THE GAME */
struct Quoridor{
	char turn_index;
	char players_pos[2][2];
	char walls_center[6][6];
	char move_mode;
	char remaining_walls[2];
	char time_left;
	char win;
	char move_confirmed;
};

struct PossibleWall{
	char orientation;
	char position[2];
	char valid;
	char visited_cells[7][7];
	char trap;
};

struct PossibleShifts{
	char up;
	char down;
	char left;
	char right;
	
};

typedef struct Quoridor Quoridor;
typedef struct PossibleWall PossibleWall;
typedef struct PossibleShifts PossibleShifts;

/* FUNTIONS FOR THE GAME */
void init_board(void);
void apply_move(void);
void update_possible_shifts(void);
void clear_possible_shifts(void);
void clear_visited_cells(void);
void view_possible_shifts(char choice);
void view_remaining_walls(void);
void view_winner_screen(void);
void view_start_screen(void);
void play(void);
char check_winner(void);
char search_goal_path(char x,char y,char player_id);
char wall_check(void);
void restore_walls(void);
void reset_wall_mode(void);

#endif
