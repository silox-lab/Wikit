#ifndef WIDGETS
#define WIDGETS
#include <ncurses.h>

void draw_border_win(WINDOW *win, int lines, int cols);
WINDOW *add_win(bool draw_border, int lines, int cols, int begin_x, int begin_y);

#endif
