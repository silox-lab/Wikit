/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#include "../include/globals.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/utils.h"
#include "../include/schema.h"
#include "../include/database/general.h"

static WinList G_WIN_LIST;
static char *W_STORAGE_PATH;
static Wiki *G_CURRENT_WIKI;
static Scope *G_CURRENT_SCOPE;
static sqlite3 *W_DATABASE;

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

char *GET_DB_PATH() {
  char *user = get_user();
  if (user == NULL) return NULL;
  
  char *path = malloc(sizeof(char) * 256);

  snprintf(path, 256, "/home/%s/wikit.db", user);
  return path;
}

char *GET_STORAGE_PATH() {
  char *user = get_user();
  if (user == NULL) return NULL;
  
  char *path = malloc(sizeof(char) * 256);

  snprintf(path, 256, "/home/%s/wikit", user);
  return path;
}

Wiki *GET_G_CURRENT_WIKI() {
  return G_CURRENT_WIKI;
}
void SET_G_CURRENT_WIKI(Wiki *wiki) {
  G_CURRENT_WIKI = wiki;
}

Scope *GET_G_CURRENT_SCOPE() {
  return G_CURRENT_SCOPE;
}
void SET_G_CURRENT_SCOPE(Scope *scope) {
  G_CURRENT_SCOPE = scope;
}

void INIT_DATABASE() {
  Result db = open_database();

  if (db.type != SUCCESS) { 
    W_DATABASE = NULL;
  } else {
    W_DATABASE = (sqlite3 *)db.value;
  }
}
sqlite3 *GET_W_DATABASE() {
  return W_DATABASE;
}