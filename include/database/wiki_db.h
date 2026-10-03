/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#ifndef WIKI_DB
#define WIKI_DB
#include "../../include/errors.h"
#include "../schema.h"
#include <stdint.h>

Result create_wiki_db(Wiki *wiki);
Result get_wiki_db(char *name, int64_t id);
Result get_all_wikis();
Result get_wiki_scopes_db(int64_t wiki_id);
Result delete_wiki_db(int64_t id);
Result update_wiki_db(char *name, Wiki w);

#endif
