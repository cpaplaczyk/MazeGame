//Created by Clayton Paplaczyk
//Description: This file contains a limited set of functions to enable drawing
//on the console in Windows and Linux.

//	      sets up the console for drawing and clears it.
//	      void console_start()
//
//	      closes down the console when in drawing mode.
//	      void console_shutdown()
//
//	      puts a character on the string at a point.
//	      void print_char(int x, int y, char c)
//
//	      puts a string on the string at a point.
//	      void print_string(int x, int y, const char* str)
//
//            returns the next key press without waiting for enter.
//	      int get_input()

//LINUX USERS: You will need to install the ncurses-dev (or similar)
//		       package. 

//these are header guards, see http://en.wikipedia.org/wiki/Include_guard
#ifndef CONSOLEUTIL_H
#define CONSOLEUTIL_H

//MACROS (VS)
#pragma warning(disable: 4996)

//MACROS (CONSTANTS)
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25

void dump_key_buffer()
{
	int c = getchar();

	while (c != '\n' && c != EOF)
		c = getchar();
}

#if defined(WIN32) || defined(_WIN32)

#include <conio.h>
#include <windows.h>

void console_activate()
{
	fflush(stdout);
}

void console_deactivate()
{
	fflush(stdout);
}

void print_char(int x, int y, char c)
{
	COORD pos = {x , y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);

	printf("%c", c);
}

void print_string(int x, int y, const char* str)
{
	COORD pos = {x , y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);

	printf("%s", str);
}

int get_input()
{
	return _getch();
}

void get_string(char* str)
{
	scanf("%s", str);
	dump_key_buffer();
}


#elif defined(__unix__) || defined(__apple__)

#include <ncurses.h>

void console_activate()

{
	initscr();
	raw();
    noecho();

	resize_term(25, 80);
}

void console_deactivate()
{
	endwin();
}

void print_char(int x, int y, char c)
{
	mvaddch(y, x, c);
	refresh();
}

void print_string(int x, int y, const char* str)
{
	mvaddstr(y, x, str);
	refresh();
}

int get_input()
{
	return getch();
}

void get_string(char* str)
{
	getstr(str);
}

#else 

#error Unsupported platform

#endif


#endif