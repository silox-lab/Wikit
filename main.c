#include <ncurses.h>
#include <locale.h>
#include "include/event_loop.h"
#include <unistd.h>

int main(void) {
  
  setlocale(LC_ALL, "");
  initscr();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  run_event_loop();
  sleep(15);

  endwin();
  return 0;
}