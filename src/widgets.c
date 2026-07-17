#define _XOPEN_SOURCE_EXTENDED 1
#include <ncurses.h>

void draw_border_win(WINDOW *win, int lines, int cols) {
    
  cchar_t lu_bend, ru_bend, ld_bend, rd_bend;

  setcchar(&lu_bend, L"╭", A_NORMAL, 0, NULL);
  setcchar(&ru_bend, L"╮", A_NORMAL, 0, NULL);
  setcchar(&ld_bend, L"╰", A_NORMAL, 0, NULL);
  setcchar(&rd_bend, L"╯", A_NORMAL, 0, NULL);

  mvwhline_set(win, 0, 1, WACS_HLINE, cols - 2);
  mvwadd_wch(win, 0, 0, &lu_bend);
  mvwadd_wch(win, 0, cols - 1, &ru_bend);

  mvwvline_set(win, 1, 0, WACS_VLINE, lines - 2);
  mvwvline_set(win, 1, cols - 1, WACS_VLINE, lines - 2);

  mvwhline_set(win, lines - 1, 1, WACS_HLINE, cols - 2);
  mvwadd_wch(win, lines - 1, 0, &ld_bend);
  mvwadd_wch(win, lines - 1, cols - 1, &rd_bend);
}

WINDOW *add_win(bool draw_border, int lines, int cols, int begin_x, int begin_y) {

  WINDOW *win = newwin(lines, cols, begin_x, begin_y);
  if (draw_border) draw_border_win(win, lines, cols);
  
  wrefresh(win);
  return win;
}
