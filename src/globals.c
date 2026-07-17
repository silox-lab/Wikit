#include "../include/utils.h"

static WinList G_WIN_LIST;
static char *WSTORAGE_PATH;

void INIT_G_WIN_LIST() {
  
  WinList win_list = {
    .capacity = 10,
    .length = 0,
    .list = malloc(sizeof(WinAndInfo) * 10),
  };

  G_WIN_LIST = win_list;
}

WinList *GET_G_WIN_LIST() {
  return &G_WIN_LIST;
}

void INIT_WSTORAGE_PATH() {}

char *GET_WSTORAGE_PATH() {
  return WSTORAGE_PATH;
}
