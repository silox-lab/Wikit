#ifndef UTILS
#define UTILS
#include <ncurses.h>
#include <stdlib.h>

typedef struct {
  WINDOW *win;
  char *tag;
} WinAndInfo;

typedef struct {
  WinAndInfo **list;
  int length;
  int capacity;
} WinList;

extern WinList *win_list;

void win_list_init();
void win_list_add(WINDOW *win, char *tag);
void win_list_free();

#endif
