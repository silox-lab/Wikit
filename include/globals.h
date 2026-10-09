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

char *GET_DB_PATH();

void INIT_DATABASE();
sqlite3 *GET_W_DATABASE();

char *GET_STORAGE_PATH();

#endif
