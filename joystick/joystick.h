																			 /*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           joystick.h
** Last modified Date:  2018-12-30
** Last Version:        V1.00
** Descriptions:        Prototypes of functions included in the lib_joystick, funct_joystick .c files
** Correlated files:    lib_joystick.c, funct_joystick.c
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

/* lib_joystick */
void joystick_init(void);
void joystick_player(char choice);
void joystick_confirm(char choice);
void joystick_confirm_wall(char x,char y,char orientation);

//EXTRAPOINT2
void select_button_mode(char button_id);
void select_button_player(char button_id);


