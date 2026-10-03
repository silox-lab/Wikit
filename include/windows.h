/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#ifndef WINDOWS
#define WINDOWS
#include "globals.h"

WinAndInfo *get_win_from_list(char *win_tag);
void win_list_init();
void win_list_add(WINDOW *win, char *tag);
void win_list_free();

#endif