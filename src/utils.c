#include "../include/utils.h"
#include <ncurses.h>
#include <stdlib.h>

WinList *win_list;

void win_list_init() {
  win_list->capacity = 10;
  win_list->length = 0;
  win_list->list = malloc(win_list->capacity * sizeof(WinAndInfo *));
}

void win_list_add(WINDOW *win, char *tag) {
  if (win_list->length >= win_list->capacity) {
    win_list->list = realloc(win_list->list, win_list->capacity * 2);
  }

  WinAndInfo *win_info = malloc(sizeof(WinAndInfo));

  win_info->win = win;
  win_info->tag = tag;

  win_list->list[win_list->length] = win_info;
  win_list->length++;
}

void win_list_free() {
  for (int i = 0; i < win_list->length; i++) {
    free(win_list[i].list);
  }
  win_list->list = NULL;
}
