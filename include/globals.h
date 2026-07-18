#ifndef GLOBALS
#define GLOBALS
#include <ncurses.h>

typedef struct {
  WINDOW *win;
  char *tag;
} WinAndInfo;

typedef struct {
  WinAndInfo **list;
  int length;
  int capacity;
} WinList;

WinList *GET_G_WIN_LIST();
void INIT_G_WIN_LIST();

char *GET_WSTORAGE_PATH();
void INIT_WSTORAGE_PATH();

#endif
