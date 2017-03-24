#include <ncurses.h>

int main(){
	initscr();
	cbreak();
	move(200,200);
	getch();
	endwin();

	return 0;

}