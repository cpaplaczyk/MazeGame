//Created by Clayton Paplaczyk

//INCLUDES
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h> 
#include "ConsoleUtil.h"
#include <cmath>

//MACROS (VS)

#pragma warning(disable: 4996)

//MACROS (CONSTANTS)

#define LEVEL_WIDTH CONSOLE_WIDTH		//use entire width of console.
#define LEVEL_HEIGHT (CONSOLE_HEIGHT-1)	//minus one, so one line free for text at bottom.

//set of ASCII characters to use as icons
#define ICON_BLANK ' '
#define ICON_WALL '#'
#define ICON_GOAL 'G'
#define ICON_PLAYER '!'

//maximum length of strings (including terminator).
#define MAX_STR_LEN 1024

//save file
#define SAVE_TIMES "times.dat"

//STRUCTURES

//stores player's position.
struct Vector
{
	//in our coordinate system, the top left character is the origin at 0.0.
	//x increases to the right while y increases downward.
	int x;
	int y;
};

//stores player's best time.
struct PlayerTime {	
	char		name[MAX_STR_LEN];
	float		seconds;
	struct PlayerTime* next;
};

//GLOBAL VARIABLES
struct PlayerTime* list_head = NULL;


//FORWARD DECLARATIONS
void menu();
void menu_selection(char option);
void besttimes_view();
void besttimes_add(char name[MAX_STR_LEN], float seconds);
void besttimes_load();
void besttimes_save();
void besttimes_free(struct PlayerTime* node);
void play();
void generate_level(char level[LEVEL_HEIGHT][LEVEL_WIDTH]);
void draw_level(char level[LEVEL_HEIGHT][LEVEL_WIDTH]);
void make_maze(char level[LEVEL_HEIGHT][LEVEL_WIDTH], int start_x, int start_y, int end_x, int end_y); 
int is_walkable(char level[LEVEL_HEIGHT][LEVEL_WIDTH], int x, int y);
void move_player(struct Vector* player_position, int move_x, int move_y);


//entry point
void main()
{
	char input_buffer; 

	printf("\nWelcome to MAZE\n");

	//load best times
	besttimes_load();	

	//menu and input loop
	do
	{
		menu();

		input_buffer = tolower(getchar()); 
		dump_key_buffer();
		menu_selection(input_buffer);
	} 
	while (input_buffer != 'q');

	//save best times and free linked list
	besttimes_save();
	besttimes_free(list_head);
}

//displays a menu containing actions the user may choose.	
void menu()
{
	printf("\nMenu Options\n");
	printf("------------------------------------------------------\n");
	printf("p: Play\n");
	printf("v: View Best Times\n");
	printf("q: Quit\n");

	printf("\n\nPlease enter a choice ---> "); 
}

//takes a character representing an inputs menu choice and calls the appropriate
//function to fulfill that choice. Display an error message if the character is
//not recognized.
void menu_selection(char option)
{
	switch(option)
	{
	case 'p':
		play();
		break;

	case 'v':
		besttimes_view();
		break;

	case 'q':
		// main loop will take care of this.
		break;

	default:
		printf("\nError: Invalid Input.  Please try again..."); 
		break;
	}
}

void besttimes_view()
{
	//TODO: Implement besttimes_view(). This function will display the contents of the
	//      list_head  linked list. If there are no entries in the list, warn the user.
	//      [5 points] Expected output example:
	//	12.235000 : Ruben
	//	27.123000 : Calvin 

	struct PlayerTime *n;
	n = list_head;

	if (n == NULL)
		printf("\nNo entries found.\n");
	else {
		while(n != NULL){
			printf("%f : %s\n\n", n->seconds, n->name);
			n = n->next;
		}
	}
}

void besttimes_add(char name[MAX_STR_LEN], float seconds)
{
	//TODO: Implement besttimes_add() so it creates a new node (using heap memory) and
	//      inserts it into the linked list. Sort the list by time (seconds) in descending
	//      order as they are inserted into the list. [10 points]
	
	struct PlayerTime *n, *temp;
	n = (struct PlayerTime *) malloc(sizeof(struct PlayerTime));
	temp = list_head;

	if(n == NULL)
		printf("Out of memory.\n");
	
	n->next = 0; //Since there is no memory there can be no players.
	
	if(list_head == 0)
		list_head = n;
	else{
		while(temp -> next != NULL)
			temp = temp->next;
		temp->next = n;
	}
}

void besttimes_load()
{
	//TODO: Implement besttimes_load(). Use C file IO to load the contents of a local
	//      file defined as SAVE_TIMES and store it in list_head. (Hint: Maybe 
	//      besttimes_add is useful.)  [10 points]

	FILE *loadTimes;
	struct PlayerTime *node, *temp;
	char pname[1024];
	float time;
	loadTimes = fopen("SAVE_TIMES", "rb");

	if(loadTimes != NULL){
		while(fread(pname, 1024, 1, loadTimes) == 1){
			fread(pname, 1024, 1, loadTimes);
			fread(&time, sizeof(float), 1, loadTimes);
			node = (struct PlayerTime*) malloc(sizeof(struct PlayerTime));
			strcpy(node->name, pname);
			node->seconds = time;
			node->next = NULL;
			if(list_head != NULL){
				temp = list_head;
				while(TRUE){
					if(temp->next == NULL)
						temp->next = node;
					
					temp = temp->next;
				}
			}
		}
	}
}

void besttimes_save()
{
	//TODO: Implement besttimes_save(). Use C file IO to save the contents of list_head
	//      to a local file defined as SAVE_TIMES. [5 points]
	FILE *saveTimes;

	saveTimes = fopen("SAVE_TIMES.dat", "wb");
	
	if(saveTimes != NULL){
		//node = list_head;
		while(list_head != NULL){
			fwrite(&list_head->seconds, sizeof(float), 1, saveTimes);
			fwrite(list_head->name, 1, sizeof(list_head->name), saveTimes);
			list_head = list_head->next;
		}
		fclose(saveTimes);
	}
	else
		printf("ERROR: Could not open file for saving data.");
}

void besttimes_free(struct PlayerTime* node)
{
	//TODO: Implement besttimes_free(). This function will free the list_head linked-list
	//      from heap memory when the program is exited. (Hint: see slide 84.) [5 points]

	if(node != NULL){
		besttimes_free(node->next);
		free(node);
	}
}

void make_maze(char level[LEVEL_HEIGHT][LEVEL_WIDTH], int start_x, int start_y, int end_x, int end_y){
	
	int solidWall = rand() % 4; //0 1 2 3 - pick the wall to leave solid
	int gapLength = 3;

	if(end_x - start_x < 10 || end_y - start_y < 10){ // to small to divide chamber
		return;
	}


		//starts to divide chambers into walls 
        int xDivide = abs(rand() % (end_x-start_x-3));
        int yDivide = abs(rand() % (end_y-start_y-3));
		if(xDivide <= 3 || yDivide <= 3)
			return;

		for(int x = start_x; x < end_x; x++){
            if(level[x][yDivide] != ICON_GOAL){ //so that ICON_GOAL does not get covered
				if(level[x][yDivide] == ICON_BLANK){ 
					level[x][yDivide] = ICON_WALL;

			//impliments the hole cutting
			//horiz left x
			if(solidWall != 0){ //start_x -> xdivide
				//int gapLength = rand() % (xDivide-start_x);
				int startPoint = rand() % ((yDivide-start_y)+start_y);
				
				//if startPoint is not in range, adjust
				if(startPoint < (start_x+1))
					startPoint+=start_x+1;
				if(startPoint > (xDivide-1))
					startPoint-=xDivide-1;

				for(int x = startPoint; x < gapLength; x++){
					level[startPoint + x][yDivide] = ICON_BLANK;
					}
				}
			//horiz right x
			if(solidWall != 1){ //xdivide -> end_x
				//int gapLength = rand() % (end_x-xDivide);
				int startPoint = rand() % ((yDivide-end_y)+end_y);

				//if startPoint is not in range, adjust
				if(startPoint < (xDivide+1))
					startPoint+=xDivide+1;
				if(startPoint > (end_x-1))
					startPoint-=end_x-1;

				for(int x = startPoint; x < gapLength; x++){
					level[startPoint + x][yDivide] = ICON_BLANK;
            }
        }
        
				}
            }
        }
        for(int y = start_y; y < end_y; y++){
			if(level[y][yDivide] != ICON_GOAL){
				if(level[xDivide][y] == ICON_BLANK){
					level[xDivide][y] = ICON_WALL;

			//implements the hole cutting
			//Vert top y
			if(solidWall != 0){ //start_y -> ydivide
				//int gapLength = rand() % (yDivide-start_y);
				int startPoint = rand() % ((xDivide-end_x)+end_x);

				//if startPoint is not in range, adjust
				if(startPoint < (start_y+1))
					startPoint+=start_y+1;
				if(startPoint > (yDivide-1))
					startPoint-=yDivide-1;

				for(int y = startPoint; y < gapLength; y++){
					level[xDivide][startPoint + y] = ICON_BLANK;
            }
        }
			//Vert lower y
			if(solidWall != 1){ //ydivide -> end_y
				//int gapLength = rand() % (end_y-yDivide);
				int startPoint = rand() % (end_y-yDivide-1);

				//if startPoint is not in range, adjust
				if(startPoint < (yDivide+1))
					startPoint+=yDivide+1;
				if(startPoint > (end_y-1))
					startPoint-=end_y-1;

				for(int y = startPoint; y < gapLength; y++){
					level[xDivide][startPoint + y] = ICON_BLANK;
            }
        }
				}
            }
        }

		//calls the make_maze method for UR, UL, LL, LR
       make_maze(level, start_x, start_y, xDivide, yDivide);
       make_maze(level, start_x, yDivide, xDivide, end_y);
       make_maze(level, xDivide, start_y, end_x, yDivide);
       make_maze(level, xDivide, yDivide, end_x, end_y);
	  
}

void play()
{
	//define the main 2d array. it has a size that is constant at compile time.
	char level[LEVEL_HEIGHT][LEVEL_WIDTH];
	
	char randomness = ' ';
	int a = LEVEL_HEIGHT, b = LEVEL_WIDTH;
	level[a][b];
	memset(level, randomness, sizeof level);

	//pick the goal to be the very last position in the map.
	int goal_x = LEVEL_WIDTH-2;
	int goal_y = LEVEL_HEIGHT-2;

	//buffer for the key pressed. initialized to a dummy value so compiler doesn't complain.
	char key_pressed = ' ';

	//for loops
	int i;

	//time for player's start and end. and name. used for best times.
	struct timeb start, end;
	char name[MAX_STR_LEN];

	//holds player's position, defaulted to 1,1.
	struct Vector player_position;
	player_position.x = 1;
	player_position.y = 1;

	//create level in the level 2D array.
	generate_level(level);

	//display the newly created level on the screen.
	console_activate();

	//prints boarder with #
//	for(i = 0; i < sizeof(level); i++){
//		level[0][i] = level[ICON_WALL][ICON_WALL];
//		level[i][0] = level[ICON_WALL][ICON_WALL];
//		level[LEVEL_WIDTH][i] = level[ICON_WALL][ICON_WALL];
//		level[i][LEVEL_HEIGHT] = level[ICON_WALL][ICON_WALL];
//	}
	//only need to do this once since the level is static.
	draw_level(level);

	//prints goal
	print_char((goal_x), (goal_y), ICON_GOAL);

	//draw the player - this is the result of calling move_player with a movement of 0,0.
	move_player(&player_position, 0, 0);

	//start the timer
	ftime(&start);

	while(key_pressed != 'q')
	{
		//waits for the user to press one key and then returns it.
		key_pressed = get_input();

		//handles player movement.
		//for each case, we check if the position the player wants to move is walkable,
		//then we move the player if it is possible.
		switch(key_pressed)
		{
		case 'w':
			if(is_walkable(level, player_position.x + 0, player_position.y - 1))
				move_player(&player_position, 0, -1);

			break;

		case 'a':
			if(is_walkable(level, player_position.x - 1, player_position.y + 0))
				move_player(&player_position, -1, 0);
			break;

		case 's':
			if(is_walkable(level, player_position.x + 0, player_position.y + 1))
				move_player(&player_position, 0, 1);
			break;

		case 'd':
			if(is_walkable(level, player_position.x + 1, player_position.y + 0))
				move_player(&player_position, 1, 0);
			break;

		default:
			//don't need to do anything.
			break;
		}

		//check if the player is at the goal and react accordingly
		if (player_position.x == goal_x && player_position.y == goal_y)
		{
			break;
		}
	}

	ftime(&end);	

	//only add high score if player didn't force quit
	if(key_pressed != 'q')
	{
		for(i = 0; i < CONSOLE_HEIGHT; i++)
			print_string(0, i, "                                                                                ");

		print_string(0, 0, "COMPLETED! Enter name (name, no spaces, then press enter.): ");

		get_string(name);

		besttimes_add(name, (end.time - start.time) +  (end.millitm - start.millitm) / 1000.0f);
	}

	console_deactivate();

	//WARNING: it seems that on linux this doesn't show the name as its entered or restore the menu correctly.
}

//generates a new level with a goal.
void generate_level(char level[LEVEL_HEIGHT][LEVEL_WIDTH])
{
	srand(time(0));
	int x, y;

	make_maze(level,1,1,LEVEL_HEIGHT,LEVEL_WIDTH);

	/*for (y = 0; y < LEVEL_HEIGHT; y++) 
		for (x = 0; x < LEVEL_WIDTH; x++)
			make_maze(level,1,1,LEVEL_HEIGHT,LEVEL_WIDTH);
			*/
		


	//int x, y;
	/*char data[LEVEL_HEIGHT][LEVEL_WIDTH] = 	 
	{{'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}, 
	{'#', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', '#', ' ', '#', '#', '#', ' ', '#', ' ', '#', '#', ' ', '#', '#', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', ' ', '#', ' ', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', '#'}, 
	{'#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', '#', '#', '#', '#', '#', ' ', '#', '#', ' ', '#', ' ', ' ', '#', ' ', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', '#', '#', '#', '#', ' ', '#', '#', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'}, 
	{'#', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', '#', '#', ' ', '#', '#', '#', ' ', '#', ' ', '#', '#', ' ', '#', '#', '#', ' ', '#', ' ', '#', ' ', '#', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', '#'}, 
	{'#', ' ', '#', ' ', '#', '#', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', ' ', '#', '#', '#', '#', ' ', '#', '#', '#', ' ', '#', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', '#', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}, 
	{'#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', '#', ' ', '#', '#', ' ', '#', '#', ' ', '#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'}, 
	{'#', '#', '#', ' ', '#', ' ', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', '#', ' ', ' ', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'}, 
	{'#', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#'}, 
	{'#', ' ', '#', ' ', '#', '#', ' ', '#', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'}, 
	{'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', '#', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'}, 
	{'#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', ' ', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}, 
	{'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#'}, 
	{'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'G', '#'}, 
	{'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}};
	*/
	//for (y = 0; y < LEVEL_HEIGHT; y++) 
		//for (x = 0; x < LEVEL_WIDTH; x++)
			//level[y][x] = data[y][x];
}

//draws the level in the console.
void draw_level(char level[LEVEL_HEIGHT][LEVEL_WIDTH])
{
	int y, x;

	for (y = 0; y < LEVEL_HEIGHT; y++) 
		for (x = 0; x < LEVEL_WIDTH; x++)
			print_char(x, y, level[y][x]);
}

//returns 1 if a location is walkable and 0 otherwise. this is a c-style boolean.
int is_walkable(char level[LEVEL_HEIGHT][LEVEL_WIDTH], int x, int y)
{
	if (level[y][x] == ICON_BLANK || level[y][x] == ICON_GOAL)	
		return 1;
	else
		return 0;	
}

//moves the player on the draw by erasing and redrawing them. adjusts the player's
//position after drawing.
void move_player(struct Vector* player_position, int move_x, int move_y)
{
	print_char(player_position->x, player_position->y, ICON_BLANK);

	player_position->x += move_x;
	player_position->y += move_y;

	print_char(player_position->x, player_position->y, ICON_PLAYER);

	//hack: move cursor out of the way
	print_char(CONSOLE_WIDTH-2, CONSOLE_HEIGHT-1, ' ');
}
