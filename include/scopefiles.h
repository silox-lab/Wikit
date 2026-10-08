/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#ifndef SCOPEFILES
#define SCOPEFILES
#include "errors.h"
#include "schema.h"

Result get_scopefile(char *path, char *name);
Result delete_scopefile(char *path, char *name);
Result build_scopefile_path(ScopeFile sf);
Result make_scopefile(ScopeFile sf);
Result update_scopefile(char *name, int64_t scope_id, char *new_name, char *new_extension);

#endif