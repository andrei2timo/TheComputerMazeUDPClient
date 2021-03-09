// TheComputerMazeUDPClient.cpp : Defines the entry point for the console application.
//
// The Computer Maze Program
//
// The Computer Maze Program finds it's way around a new Computing building called Y-block
// on it's way around it must open locked doors, read books, attend events, collect valuable items and carry out other tasks
//
// Change Control 
// Name: 	   | Date:      | Reason for change
// Andrei Timo | 12/03/2020 | Original Version
// Andrei Timo | 20/03/2020 | Managing the backpack better
// Andrei Timo | 27/03/2020 | Quality of Code improvements
//
//
#include "stdafx.h"
#include <winsock2.h>
#include <time.h>
#include <conio.h>

#pragma comment(lib, "wsock32.lib")

#define STUDENT_NUMBER		"19000915"
#define STUDENT_FIRSTNAME	"Andrei-Claudiu"
#define STUDENT_FAMILYNAME	"Timo"

#define IP_ADDRESS_SERVER	"127.0.0.1"
//#define IP_ADDRESS_SERVER "164.11.80.69"


#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define MAX_FILENAME_SIZE 500

#define MAX_BUFFER_SIZE   5000
#define MAX_STRING_SIZE   200
#define MAX_NO_TOKENS     50

#define MAX_ITEMS_IN_ROOM		20
#define MAX_ITEMS_IN_BACKPACK	50

#define OPTION_MOVE_NORTH 1
#define OPTION_MOVE_SOUTH 2
#define OPTION_MOVE_EAST 3
#define OPTION_MOVE_WEST 4
#define OPTION_MOVE_UP 5
#define OPTION_MOVE_DOWN 6

#define OPTION_UNLOCK_NORTH 7
#define OPTION_UNLOCK_SOUTH 8
#define OPTION_UNLOCK_EAST 9 
#define OPTION_UNLOCK_WEST 10
#define OPTION_UNLOCK_UP 11
#define OPTION_UNLOCK_DOWN 12

#define OPTION_BASE_FOR_READS 200
#define OPTION_BASE_FOR_PICKUPS 500
#define OPTION_BASE_FOR_DROPS 800
#define OPTION_BASE_FOR_DOS 1100
#define OPTION_BASE_FOR_EVENTS 1300

enum directions
{
	DIRECTION_NORTH = 0,
	DIRECTION_SOUTH = 1,
	DIRECTION_EAST = 2,
	DIRECTION_WEST = 3,
	DIRECTION_UP = 4,
	DIRECTION_DOWN = 5
};


enum direction_possible
{
	DIRECTION_NOT_PRESENT = -1,
	DIRECTION_LOCKED = 0,
	DIRECTION_OPEN = 1
};


enum item_types
{
	ITEM_NONE = 0,
	ITEM_BOOK = 1,
	ITEM_JUNK = 2,
	ITEM_EQUIPMENT = 3,
	ITEM_MANUSCRIPT = 4,
	ITEM_TEST = 5,
	ITEM_OTHER = 10
};


// Structure Name : Item
// Struct members :	number - Variable Type : Int
//                   value - Variable Type : Int
//                  volume - Variable Type : Int 
struct Item
{
	int  number;
	int  value;
	int  volume;
};

// Structure Name : Student
// Struct members :	level - Variable Type : Int
//          rooms_visited - Variable Type : Int
//          doors_openned - Variable Type : Int 
//        number_of_moves - Variable Type : Int 
//                  score - Variable Type : Int 
struct Student
{
	int level;
	int rooms_visited;
	int doors_openned;
	int number_of_moves;
	int score;
};

// Structure Name : Room
// Struct members : name - Variable Type : Char
//					type - Variable Type : Int
//			   direction - Variable Type : Int (Array) 
//        number_of_keys - Variable Type : Int
//             keys      - Variable Type : Int (Array) 
//       number_of_items - Variable Type : Int 
//                 items - Variable Type : Item (Array)
struct Room
{
	char name[5];
	int  type;
	int  direction[6];
	int  number_of_keys;
	int  keys[4];
	int  number_of_items;
	Item items[MAX_ITEMS_IN_ROOM];
};

// Structure Name : BackPack
// Struct members :	number_of_items - Variable Type : Int
//                            items - Variable Type : Item
struct Backpack
{
	int number_of_items;
	Item items[MAX_ITEMS_IN_BACKPACK];
};




#define MAX_OPTIONS	50

int number_of_options;
int options[MAX_OPTIONS];


Student student;
Room room;
Backpack backpack;



SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;




char InputBuffer[MAX_BUFFER_SIZE];

char Tokens[MAX_NO_TOKENS][MAX_STRING_SIZE];

char text_student[1000];
char text_backpack[1000];
char text_room[1000];
char text_keys[1000];
char text_items[1000];
char text_options[1000];


// sentOption()
// This function sends a new option to the Computer Maze Marking Server
// Inputs:  option - this is the move that we would like to make
//          key - this is the key to use if we are unlocking a door
// Outputs: None
// Warning: None
//
//
void sentOption(int option, int key)
{
	char buffer[100];

	sprintf(buffer, "Option %d, %x", option, key);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));
}

#define NO_MOVE -1
/*************************************************************/
/********* Your tactics code starts here *********************/
/*************************************************************/

#define KNOWN_KEYS 20
#define NUMBER_OF_WORKIN_KEYS 58
struct WorkingKeys
{
	char room_name[5];
	int direction;
	int key;
};
WorkingKeys working_keys[NUMBER_OF_WORKIN_KEYS] =
{
	{"0Y03", 2, 0x200F},
	{"0Y20", 0, 0xA100},
	{"0Y20", 2, 0x29FC},
	{"0Y33", 1, 0xBE27},
	{"0Y33", 2, 0xFD5A},
	{"0Y40", 0, 0xD75B},
	{"0Y50", 1, 0x71DA},
	{"0Y51", 3, 0xC567},
	{"0Y55", 1, 0x200F},
    {"0Y61", 2, 0xBCD5},
    {"0Y62", 0, 0x8FD4},
    {"0Y73", 1, 0x090D},
    {"0Y73", 3, 0x8E28},
    {"0Y82", 1, 0xBCD5},
    {"0Y84", 0, 0xA100},
    {"0Y92", 2, 0x9F4A},
    {"1Y11", 0, 0x8E28},
    {"1Y23", 2, 0xBCD5},
    {"1Y51", 0, 0x29FC},
    {"1Y51", 2, 0x8FD4},
    {"1Y52", 1, 0x29FC},
    {"1Y53", 0, 0xE4A8},
    {"1Y61", 2, 0xA100},
    {"1Y63", 2, 0xBCD5},
    {"1Y71", 0, 0x29FC},
    {"1Y72", 2, 0x8FD4},
    {"1Y73", 0, 0x090D},
    {"1Y80", 2, 0x8FD4},
    {"2Y00", 1, 0x0172},
    {"2Y00", 2, 0x71DA},
    {"2Y10", 2, 0xC567},
    {"2Y15", 2, 0x29FC},
    {"2Y17", 2, 0x8E28},
    {"2Y18", 2, 0xA100},
    {"2Y22", 2, 0x8E28},
    {"2Y31", 3, 0xC567},
    {"2Y32", 1, 0xD75B},
    {"2Y32", 2, 0x29FC},
    {"2Y35", 0, 0x71DA},
    {"2Y41", 0, 0xFD5A},
    {"2Y52", 3, 0x9F4A},
    {"2Y53", 1, 0xBE27},
    {"2Y71", 2, 0x8E28},
    {"2Y73", 1, 0xD2F7},
    {"2Y82", 0, 0x200F},
    {"2Y92", 0, 0x71DA},
    {"2Y92", 2, 0x71DA},
    {"3Y01", 3, 0xA100},
    {"3Y03", 1, 0x0172},
    {"3Y14", 1, 0x8FD4},
    {"3Y18", 1, 0xD75B},
    {"3Y19", 3, 0xBCD5},
    {"3Y25", 0, 0xFD5A},
    {"3Y25", 2, 0x8E28},
    {"3Y28", 2, 0x8FD4},
    {"3Y30", 0, 0x090D},
    {"3Y32", 3, 0x090D},
    {"3Y37", 3, 0xD2F7}
};
int known_keys[KNOWN_KEYS] = { 0x200F, 0xBCD5, 0xE4A8, 0x71DA,
							  0x29FC, 0xA100, 0xBE27, 0x9F4A,
							  0x8E28, 0xD75B, 0x090D, 0x0172,
							  0xD2F7, 0xC567, 0x8FD4, 0xFD5A,
						      0x8F32, 0x1AE1, 0x0EB9, 0x43A1};

int option_count = 0;
char room_name[10] = " ";

int rooms_visited[5][10][10];
int doors_tried[5][10][10][4];
int keys_tried[5][10][10][4];

int use_key;

int try_key = -1;
int last_action = -1;
// initRooms()
// This function initializes the 3 arrays (rooms_visited, doors_tried and keys_tried) to be used in the program.
// Inputs:  None
// Outputs: None
// Warning: None
//
void initRooms()
{
	int floor;
	int ns;
	int ew;
	int door;
	for (floor = 0; floor < 5; floor++)
	{
		for (ns = 0; ns < 10; ns++)
		{
			for (ew = 0; ew < 10; ew++)
			{
				rooms_visited[floor][ns][ew] = 0;
				for (door = 0; door < 4; door++)
				{
					doors_tried[floor][ns][ew][door] = -1;
					keys_tried[floor][ns][ew][door] = 0;
				}
			}
		}
	}
}
// bestDirection()
// This function checks each direction and is looking for a best direction
// Inputs:  None
// Outputs: best_direction - This one is the best direction in which to move in the program
// Warning: None
//
int bestDirection()
{
	int best_direction = NO_MOVE; // Initialise to indicate no direction yet chosen
	int room_visits = 20000;     // Initialise this to a maximum number that can't ever be reached
	int floor;
	int ns;
	int ew;
	if (sscanf(room.name, "%1dY%1d%1d", &floor, &ns, &ew) == 3) // Parse the room name to get floor, ns and ew variables
	{
		if (room.direction[DIRECTION_NORTH] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns - 1][ew] < room_visits) // Comparing how many times we have moved north from here
			{
				room_visits = rooms_visited[floor][ns - 1][ew]; // So far north is the least visited direction
				best_direction = OPTION_MOVE_NORTH;
			}
		}
		if (room.direction[DIRECTION_SOUTH] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns + 1][ew] < room_visits) // Comparing how many times we have moved south from here
			{
				room_visits = rooms_visited[floor][ns + 1][ew]; // So far south is the least visited direction
				best_direction = OPTION_MOVE_SOUTH;
			}
		}
		if (room.direction[DIRECTION_EAST] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns][ew + 1] < room_visits) // Comparing how many times we have moved east from here
			{
				room_visits = rooms_visited[floor][ns][ew + 1]; // So far east is the least visited direction
				best_direction = OPTION_MOVE_EAST;
			}
		}
		if (room.direction[DIRECTION_WEST] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns][ew - 1] < room_visits) // Comparing how many times we have moved west from here
			{
				room_visits = rooms_visited[floor][ns][ew - 1]; // So far west is the least visited direction
				best_direction = OPTION_MOVE_WEST;
			}
		}
		if (room.direction[DIRECTION_UP] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor + 1][ns][ew] < room_visits) // Comparing how many times we have moved up from here
			{
				room_visits = rooms_visited[floor + 1][ns][ew]; // So far up is the least visited direction
				best_direction = OPTION_MOVE_UP;
			}
		}
		if (room.direction[DIRECTION_DOWN] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor - 1][ns][ew] < room_visits) // Comparing how many times we have moved down from here
			{
				room_visits = rooms_visited[floor - 1][ns][ew]; // So far down is the least visited direction
				best_direction = OPTION_MOVE_DOWN; 
			}
		}
	}
	return best_direction;
}
// useWorkingKeys()
// This function uses workin_keys structure array to unlock the doors
// Inputs:  use_key - the use_key parameter represents the key with which the door is unlocked
// Outputs: move - If the door has been unlocked, we will move to the next room
// Warning: None
//
int useWorkingKeys(int* use_key)
{
	int move = NO_MOVE;  // default is no key found
	int i;
	int door;
	for (i = 0; i < NUMBER_OF_WORKIN_KEYS; i++)
	{
		door = working_keys[i].direction;
		if (DIRECTION_LOCKED == room.direction[door])
		{
			if (strcmp(room.name, working_keys[i].room_name) == 0) // if the name of the room we are in and the room stored in workin_keys coincide, then we unlock the door
			{
				*use_key = working_keys[i].key;
				move = OPTION_UNLOCK_NORTH + door; // we loop through all four doors 
				return move;
			}
		}
	}
	return move;
}
//FILE* ofp;// Create a txt file
// writekeys2file()
// This function helps building workin_keys array, placing room name, (floor, "Y", ns, ew), key, door and direction into a text file
// Inputs:  key   - represents the key with which the door is unlocked
//          floor - the floor we are in at the moment
//          ns    - position N-S in the program
//          ew    - position E-W in the program
//          door  - the door that was unlocked (the room where the door is located)
//          dir   - door position (an integer value in the range [0,3]) represented by the cardinal points - N, S, E or W
// Outputs: None
// Warning: None
//
/*void writekeys2file(int key, int floor, int ns, int ew, int door, int dir)
{
	if ((ofp = fopen("mykey.txt", "a")) != NULL)// opening an existing file
	{
		fprintf(ofp, "{%dY%d%d,", floor, ns, ew);
		fprintf(ofp, "%d,", dir);
		fprintf(ofp, "0x%4X}", key);
		fprintf(ofp, ",\n");
		fclose(ofp);
	}
}*/
// attendEvent()
// This function helps us to attend  an event
// Inputs:  None
// Outputs: move - if we find a valid event, then we attend it
// Warning: None
//
int attendEvent()
{
	int move = NO_MOVE; // default is no event found
	int i;
	int option;
	for (i = 0; i < number_of_options; i++)
	{
		option = options[i];
		if (option >= OPTION_BASE_FOR_EVENTS)  // then this is a valid event
		{
			move = option;
			return move;
		}
	}
	return move;
}
// unlockDoor()
// This function unlocks the doors using known_keys array (we place keys found in different rooms in this array)
// Inputs:  use_key - represents the key with which the door is unlocked
// Outputs: move - If the door has been unlocked, we will move to the next room
// Warning: None
//
/*int unlockDoor(int* use_key)
{
	int door;
	int move = NO_MOVE;
	int key;
	int floor;
	int ns;
	int ew;
	if (sscanf(room.name, "%1dY%1d%1d", &floor, &ns, &ew) == 3)
	{
		// Check to see if we have unlocked a door...
		switch (last_move)
		{
		case OPTION_UNLOCK_NORTH:
			if (room.direction[DIRECTION_NORTH] == DIRECTION_OPEN)
			{
				// save use_key
				printf("%s, %d, 0x%04X\n", room.name, DIRECTION_NORTH, last_key);
				writekeys2file(last_key, floor, ns, ew, DIRECTION_NORTH);
				last_move = 0;
				//getchar();
			}
			break;
		case OPTION_UNLOCK_SOUTH:
			if (room.direction[DIRECTION_SOUTH] == DIRECTION_OPEN)
			{
				// save use_key
				printf("%s, %d, 0x%04X\n", room.name, DIRECTION_SOUTH, last_key);
				writekeys2file(last_key, floor, ns, ew, DIRECTION_SOUTH);
				last_move = 0;
				//getchar();
			}
			break;
		case OPTION_UNLOCK_EAST:
			if (room.direction[DIRECTION_EAST] == DIRECTION_OPEN)
			{
				// save use_key
				printf("%s, %d, 0x%04X\n", room.name, DIRECTION_EAST, last_key);
				writekeys2file(last_key, floor, ns, ew, DIRECTION_EAST);
				last_move = 0;
				//getchar();
			}
			break;
		case OPTION_UNLOCK_WEST:
			if (room.direction[DIRECTION_WEST] == DIRECTION_OPEN)
			{
				// save use_key
				printf("%s, %d, 0x%04X\n", room.name, DIRECTION_WEST, last_key);
				writekeys2file(last_key, floor, ns, ew, DIRECTION_WEST);
				last_move = 0;
				//getchar();
			}
			break;
		default:
			break;
		}
		// Try to unlock a door...
		for (door = 0; door < 4; door++)
		{
			if (room.direction[door] == DIRECTION_LOCKED)
			{
				doors_tried[floor][ns][ew][door]++;
				key = doors_tried[floor][ns][ew][door];
				if (key < KNOWN_KEYS)
				{
					move = OPTION_UNLOCK_NORTH + door;
					//sentOption(move, known_keys[key]);
					*use_key = known_keys[key];
					printf("Move = %d, key = 0x%4X\n", move, known_keys[key]);
					keys_tried[floor][ns][ew][door] = known_keys[key];
					last_key = known_keys[key];
					return move;
				}
			}
		}
	}
	return move;
}*/
// saveKeys()
// This function displays on the screen the keys found in the rooms and saved
// Inputs:  None
// Outputs: None
// Warning: None
//
void saveKeys()
{
	int floor;
	int ns;
	int ew;
	int door;
	for (floor = 0; floor < 5; floor++)
	{
		for (ns = 0; ns < 10; ns++)
		{
			for (ew = 0; ew < 10; ew++)
			{
				for (door = 0; door < 4; door++)
				{
					//if ((keys_tried[floor][ns][ew][door] > 0) && (room.direction[door] == DIRECTION_OPEN))
					if ((doors_tried[floor][ns][ew][door] >= 0) && (doors_tried[floor][ns][ew][door] < KNOWN_KEYS) && (room.direction[door] == DIRECTION_OPEN))
					{
						printf("Room %dY%d%d Door %d Key 0x%4X\n", floor, ns, ew, door, keys_tried[floor][ns][ew][door]);
					}
				}
			}
		}
	}
	getchar();
	getchar();
	getchar();
	getchar();
}
// getRandomMove()
// This function returns a random movement that we do in the Maze
// Inputs:  None
// Outputs: move - If a valid move was found, then we would move in the Y-block rooms
// Warning: None
//
int getRandomMove()
{
	int move;

	if (strcmp(room_name, room.name) != 0) option_count = 0;
	move = options[option_count];
	option_count = (option_count + 1) % number_of_options;
	return move;
}
// inPickUpList()
// This function checks if we have found an item that we can place in the backpack
// Inputs:  item_number - represents the code of the item found in room X
// Outputs: rc - is a bool type variable that tells us whether we found an item (true) or not (false)
// Warning: None
//
bool inPickUpList(int item_number)
{
	bool rc = false;
	int i;
	for (i = 0; i < number_of_options; i++)
	{
		if (options[i] == (OPTION_BASE_FOR_PICKUPS + item_number)) rc = true;
	}
	return rc;
}
// CheckItems()
// This function checks the items placed in the backpack and places those with the maximum value and volume and throws those with the minimum value.
// Inputs:  None
// Outputs: move - place the items with maximum value in the backpack, otherwise return NO_MOVE
// Warning: None
//
int CheckItems()
{
	int found_item;
	int found_value;
	int found_volume;
	float found_ratio;

	int backpack_item;
	int backpack_value;
	int backpack_volume;
	float backpack_ratio;

	int best_found_item = 0;
	int best_found_value = 0;
	int best_found_volume = 0;
	float best_found_ratio = -1.0;

	int worse_backpack_item = 0;
	int worse_backpack_value = 1000;
	int worse_backpack_volume = 1;
	float worse_backpack_ratio = 1000.0;
	int i, j;
	int move = NO_MOVE;
	if (room.number_of_items > 0)
	{ 
		for (i = 0; i < room.number_of_items; i++)
		{
			found_item = room.items[i].number;
			found_value = room.items[i].value;
			found_volume = room.items[i].volume;
			found_ratio = (float)found_value / (float)found_volume;

			if (best_found_ratio < found_ratio)
			{
				best_found_item = found_item;
				best_found_value = found_value;
				best_found_volume = found_volume;
				best_found_ratio = found_ratio;
			}
		}
		if (best_found_value > 0)
		{
			if (inPickUpList(best_found_item))//If we found an item with maximum volume and value
			{
				return OPTION_BASE_FOR_PICKUPS + best_found_item;// we can pick-up an item because our backpack isn't full
			}
			else
			{
				if (backpack.number_of_items > 0) // throw away bad items from backpack
				{
					for (j = 0; j < backpack.number_of_items; j++)
					{
						backpack_item = backpack.items[i].number;
						backpack_value = backpack.items[i].value;
						backpack_volume = backpack.items[i].volume;
						backpack_ratio = (float)backpack_value / (float)backpack_volume;

						if (worse_backpack_ratio > backpack_ratio)
						{
							worse_backpack_item = backpack_item;
							worse_backpack_value = backpack_value;
							worse_backpack_volume = backpack_volume;
							worse_backpack_ratio = backpack_ratio;
						}
					}
					if (worse_backpack_ratio < found_ratio) return OPTION_BASE_FOR_DROPS + worse_backpack_item;
				}
			}
	    }
	}
	return move;
}
// ItemValue()
// This function returns the value of an item found
// Inputs:  item - the value of a random item
// Outputs: return_value - returns the value of an item found in the room, otherwise its value remains unchanged and it will be returned (-1)
// Warning: None
//
int ItemValue(int item)
{
	int return_value = -1; //default value isn't very good
	int i;
	for (i = 0; i < room.number_of_items; i++)
	{
		if (item == room.items[i].number)
			return_value = room.items[i].value;
	}
	return return_value;
}
// pickupStuff()
// This function places all the items found in a room in the backpack, without verifying their value and volume.
// Inputs:  None
// Outputs: move - if an item was found, then we collect it using the OPTION for PICKUPS
// Warning: None
//
/*int pickupStuff()
{
	int move = NO_MOVE;
	int i;
	if (room.number_of_items > 0)
	{
		for (i = 0; i < room.number_of_items; i++)
		{   
			if (room.items[i].value > 0)
			{
				printf("Move = %d\n", OPTION_BASE_FOR_PICKUPS + room.items[i].number);
				move = OPTION_BASE_FOR_PICKUPS + room.items[i].number;
				return move;
			}
		}
	}
	return move;
}*/
// ReadBook()
// This function helps the player read the books found, checking their value.
// Inputs:  None
// Outputs: move - if the item found is a book, then we read it
// Warning: None
//
int ReadBook()
{
	int move = NO_MOVE;// default value isn't very good
	int i;
	int option;
	int item;
	for (i = 0; i < number_of_options; i++)
	{   
		option = options[i];
		// if our option is between 200 and 500 (if it is an item and can be read , then must be a book)
		if ((option >= OPTION_BASE_FOR_READS) && (option <= OPTION_BASE_FOR_PICKUPS))
		{
			item = option - OPTION_BASE_FOR_READS;
			if (ItemValue(item) > 0)// check that it has value
			{
				move = option;
				return move;
			}
		}
	}
	return move;
}
// check_unlocked()
// This function checks if the door has been unlocked to store the last key used to unlock it.
// Inputs:  None
// Outputs: None
// Warning: None
//
/*void check_unlocked()
{
	int unlock = false; // We assume that the door has not been unlocked
	// If the last_action was the attempt to unlock a door in the 4 directions (North, South, East, West)
	if (last_action >= OPTION_UNLOCK_NORTH && last_action <= OPTION_UNLOCK_WEST)
	{
		printf("Checking if unlock worked\n");
		switch (last_action)
		{
		case OPTION_UNLOCK_NORTH:
			if (room.direction[DIRECTION_NORTH] == DIRECTION_OPEN)
				unlock = true;
			break;
		case OPTION_UNLOCK_SOUTH:
			if (room.direction[DIRECTION_SOUTH] == DIRECTION_OPEN)
				unlock = true;
			break;
		case OPTION_UNLOCK_EAST:
			if (room.direction[DIRECTION_EAST] == DIRECTION_OPEN)
				unlock = true;
			break;
		case OPTION_UNLOCK_WEST:
			if (room.direction[DIRECTION_WEST] == DIRECTION_OPEN)
				unlock = true;
			break;
		default:
			break;
		}
		if (unlock == true)
		{
			printf("!!!WE UNLOCKED THE DOOR WITH KEY %x\n", use_key);
			saveKeys();
		}
		else
		{
			printf("Door did not unlock\n");
		}
	}
}*/
void yourMove()
{
	int move = NO_MOVE;  // no valid move assigned yet
	int i;
	char chr;
	int floor;
	int ns;
	int ew;
	
	if (move == NO_MOVE) move = ReadBook();//Check if the item found is a book and read it
	if (move == NO_MOVE) move = attendEvent();// Attend an event and level up
	if (move == NO_MOVE) move = CheckItems();// Check items and place the best of them in the backpack
	if (move == NO_MOVE) move = useWorkingKeys(&use_key); // Didn't pick anything up, so try unlocking a door
	
	if (sscanf(room.name, "%1dY%1d%1d", &floor, &ns, &ew) == 3) // Parse the room name to get floor, ns and ew variables
	{
		rooms_visited[floor][ns][ew]++;  // record where you have been
		if (move == NO_MOVE) move = bestDirection(); // it didn't try to unlock a door, so move in best direction
	}
	if (move == NO_MOVE) move = getRandomMove(); // always returns a valid move

    if(student.number_of_moves < 2000)//If I hadn't reached 2000 moves, sentOption to move in the maze
	    sentOption(move, use_key);

	printf("Move = %d\n", move);

	// Check whether a key from the keyboard is pressed or not
	if (_kbhit())
	{
		chr = getchar();// We store the key pressed in the variable chr
		if (chr == 's')
		{
			saveKeys();
		}
	}
	
}
/*************************************************************/
/********* Your tactics code ends here ***********************/
/*************************************************************/
int getTokens(char* instring, char seperator)
{
	int  number_of_tokens;
	char chr;
	int  state;
	int  i;
	int  j;


	for (i = 0; i < MAX_NO_TOKENS; i++)
	{
		for (j = 0; j < MAX_STRING_SIZE; j++)
		{
			Tokens[i][j] = '\0';
		}
	}

	number_of_tokens = -1;
	chr = instring[0];
	state = 0;
	i = 0;

	while (chr != '\0')
	{
		switch (state)
		{
		case 0:  // Initial state
			if (chr == seperator)
			{
				number_of_tokens++;
				state = 1;
			}
			else if ((chr == ' ') || (chr == '\t') || (chr == '\n'))
			{
				state = 1;
			}
			else
			{
				number_of_tokens++;
				j = 0;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		case 1:  // Leading white space
			if (chr == seperator)
			{
				number_of_tokens++;
				state = 1;
			}
			else if ((chr == ' ') || (chr == '\t') || (chr == '\n'))
			{
				state = 1;
			}
			else
			{
				number_of_tokens++;
				j = 0;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		case 2:  // Collecting chars
			if (chr == seperator)
			{
				//number_of_tokens++;
				state = 1;
			}
			else
			{
				j++;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		default:
			break;
		}

		i++;
		chr = instring[i];
	}

	return (number_of_tokens + 1);
}
bool getline(FILE* fp, char* buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);

		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}
void printRoom()
{
	int i;

	printf("Room\n");
	printf("Room = %s, Room type = %d\n", room.name, room.type);

	printf("Directions = ");
	for (i = 0; i < 6; i++)
	{
		printf("%d  ", room.direction[i]);
	}
	printf("\n");

	if (room.number_of_keys > 0)
	{
		printf("Keys = ");
		for (i = 0; i < room.number_of_keys; i++)
		{
			printf("0x%X  ", room.keys[i]);
		}
		printf("\n");
	}
	else
	{
		printf("No keys in this room\n");
	}

	if (room.number_of_items > 0)
	{
		for (i = 0; i < room.number_of_items; i++)
		{
			printf("Item=%d, Value=%d, Volume=%d\n", room.items[i].number, room.items[i].value, room.items[i].volume);
		}
	}
	else
	{
		printf("No items in this room\n");
	}

	printf("\n");
}
void printStudent()
{
	printf("Student\n");
	printf("Level=%d,  Number of rooms visited = %d,  Number of doors openned = %d,  Number of moves = %d,  Score = %d\n", student.level, student.rooms_visited, student.doors_openned, student.number_of_moves, student.score);
	printf("\n");
}
void printBackpack()
{
	int i;

	printf("Backpack\n");

	if (backpack.number_of_items > 0)
	{
		for (i = 0; i < backpack.number_of_items; i++)
		{
			printf("Item=%d, Value=%d, Volume=%d\n", backpack.items[i].number, backpack.items[i].value, backpack.items[i].volume);
		}
	}
	else
	{
		printf("Your backpack is empty\n");
	}
	printf("\n");
}


void printOptions()
{
	int i;

	printf("Options\n");
	printf("Options = ");
	for (i = 0; i < number_of_options; i++)
	{
		printf("%d  ", options[i]);
	}
	printf("\n");
	printf("\n");
}




void communicate_with_server()
{
	char buffer[4096];
	int  len = sizeof(SOCKADDR);
	int  i;
	char* p;
	int	 number_of_tokens;


	sprintf_s(buffer, "Register  %s %s %s", STUDENT_NUMBER, STUDENT_FIRSTNAME, STUDENT_FAMILYNAME);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));

	while (true)
	{
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&client_addr, &len) != SOCKET_ERROR)
		{
			p = ::inet_ntoa(client_addr.sin_addr);

			if ((strcmp(IP_ADDRESS_SERVER, "127.0.0.1") == 0) || (strcmp(IP_ADDRESS_SERVER, p) == 0))
			{
				printf("%s\n\n", buffer);

				number_of_tokens = getTokens(buffer, '|');

				if (number_of_tokens == 6)
				{
					strcpy(text_student, Tokens[0]);
					strcpy(text_backpack, Tokens[1]);
					strcpy(text_room, Tokens[2]);
					strcpy(text_keys, Tokens[3]);
					strcpy(text_items, Tokens[4]);
					strcpy(text_options, Tokens[5]);

					printf("Student  = '%s'\n", text_student);
					printf("Backpack = '%s'\n", text_backpack);
					printf("Room     = '%s'\n", text_room);
					printf("Keys     = '%s'\n", text_keys);
					printf("Items    = '%s'\n", text_items);
					printf("Options  = '%s'\n", text_options);

					student.level = -1;
					student.rooms_visited = -1;
					student.doors_openned = -1;
					student.number_of_moves = -1;

					if (sscanf(text_student, "%d,%d,%d,%d,%d", &student.level, &student.rooms_visited, &student.doors_openned, &student.number_of_moves, &student.score) == 5)
					{
					}

					if (strlen(text_backpack) > 0)
					{
						backpack.number_of_items = getTokens(text_backpack, '&');

						if (backpack.number_of_items > 0)
						{
							for (i = 0; i < backpack.number_of_items; i++)
							{
								if (i < MAX_ITEMS_IN_BACKPACK)
								{
									backpack.items[i].number = -1;

									if (sscanf(Tokens[i], "%d, %d, %d", &backpack.items[i].number, &backpack.items[i].value, &backpack.items[i].volume) == 3)
									{
									}
								}
							}
						}
					}
					else
					{
						backpack.number_of_items = 0;
					}

					sscanf(text_room, "%s ,%d, %d, %d, %d, %d, %d, %d", &room.name, &room.type, &room.direction[DIRECTION_NORTH], &room.direction[DIRECTION_SOUTH], &room.direction[DIRECTION_EAST], &room.direction[DIRECTION_WEST], &room.direction[DIRECTION_UP], &room.direction[DIRECTION_DOWN]);

					if (strlen(text_keys) > 0)
					{
						room.number_of_keys = getTokens(text_keys, '&');

						if (room.number_of_keys > 0)
						{
							for (i = 0; i < room.number_of_keys; i++)
							{
								if (i < 4)
								{
									room.keys[i] = -1;

									if (sscanf(Tokens[i], "%x", &room.keys[i]) == 1)
									{
									}
								}
							}
						}
					}
					else
					{
						room.number_of_keys = 0;
					}

					if (strlen(text_items) > 0)
					{
						room.number_of_items = getTokens(text_items, '&');

						if (room.number_of_items > 0)
						{
							for (i = 0; i < room.number_of_items; i++)
							{
								if (i < MAX_ITEMS_IN_ROOM)
								{
									room.items[i].number = -1;

									if (sscanf(Tokens[i], "%d, %d, %d", &room.items[i].number, &room.items[i].value, &room.items[i].volume) == 3)
									{
									}
								}
							}
						}
					}
					else
					{
						room.number_of_items = 0;
					}

					if (strlen(text_options) > 0)
					{
						number_of_options = getTokens(text_options, ',');

						if (number_of_options > 0)
						{
							for (i = 0; i < number_of_options; i++)
							{
								if (i < MAX_OPTIONS)
								{
									options[i] = -1;

									if (sscanf(Tokens[i], "%d", &options[i]) == 1)
									{
									}
								}
							}
						}
					}
					else
					{
						number_of_options = 0;
					}
				}

				printStudent();
				printBackpack();
				printRoom();
				printOptions();

				//system("timeout /t 60");

				yourMove();
			}
		}
		else
		{
			printf_s("recvfrom error = %d\n", WSAGetLastError());
		}
	}

	printf_s("Student %s\n", STUDENT_NUMBER);
}




int main()
{
	char chr = '\0';

	printf("\n");
	printf("The Computer Maze Student Program\n");
	printf("UWE Computer and Network Systems Assignment 2 \n");
	printf("\n");

	initRooms();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	//sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	//if (!sock)
	//{	
	//	printf("Socket creation failed!\n"); 
	//}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock)
	{
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	//int ret = bind(sock_recv, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	////	int ret = bind(sock_send, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	//if (ret)
	//{
	//	printf("Bind failed! %d\n", WSAGetLastError());
	//}

	communicate_with_server();

	closesocket(sock);
	WSACleanup();

	while (chr != '\n')
	{
		chr = getchar();
	}

	return 0;
}
