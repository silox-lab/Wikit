/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#ifndef WIKIS
#define WIKIS

#include "errors.h"

Result make_wiki(char *name);
Result get_wiki(char *name);
Result delete_wiki(char *name);

#endif