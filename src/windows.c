#include "../include/utils.h"
#include "../include/globals.h"
#include <string.h>

WinAndInfo *get_win_from_list(char *win_tag) {

  WinList *win_list = GET_G_WIN_LIST();
  
  WinAndInfo *found_win = NULL;
  
  for (int i = 0; i < win_list->length; i ++) {
    if (strcmp(win_list->list[i]->tag, win_tag)) {
      found_win = win_list->list[i];
      break;
    } 
  }

  return found_win;
}
