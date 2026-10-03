/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#ifndef GLOBALS
#define GLOBALS
#include "schema.h"
#include <ncurses.h>
#include <sqlite3.h>

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

char *GET_DB_PATH();

Wiki *GET_G_CURRENT_WIKI();
void SET_G_CURRENT_WIKI(Wiki *wiki);

Scope *GET_G_CURRENT_SCOPE();
void SET_G_CURRENT_SCOPE(Scope *scope);

void INIT_DATABASE();
sqlite3 *GET_W_DATABASE();

char *GET_STORAGE_PATH();

#endif
