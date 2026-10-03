/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#ifndef SCOPEFILE_DB
#define SCOPEFILE_DB
#include "../../include/errors.h"
#include "../schema.h"
#include <stdint.h>

Result create_scopefile_db(ScopeFile *scopefile);
Result delete_scopefile_db(int64_t id);
Result get_scopefile_db(char *scopefile_name, int64_t scope_id);

#endif