/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#ifndef SCOPES
#define SCOPES

#include "errors.h"
#include "schema.h"
#include <stdint.h>

Result get_scope(int64_t id);
Result make_scope(Scope s);
Result delete_scope(Scope s);
Result build_scope_path(Scope s);

#endif