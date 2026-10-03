/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#include "../../include/errors.h"
#include "../../include/globals.h"
#include <stdint.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

Result open_database() {
  printf("OPENING DATABASE: %s\n", GET_DB_PATH());
  sqlite3 *db;
  int open_stat = sqlite3_open(GET_DB_PATH(), &db);
  if (open_stat != SQLITE_OK) {
    return (Result){.type = SIMPLE_ERR,
                    .value = "failed to open or create the database."};
  }

  int fk_enable = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
  if (fk_enable != SQLITE_OK) {
    return (Result){.type = SIMPLE_ERR,
                    .value = "failed to open or create the database."};
  }
  
  return (Result){.type = SUCCESS, .value = db};
}

Result create_table_db(char *table) {

  sqlite3 *db = GET_W_DATABASE();

  char *create_err;
  int create_stat = sqlite3_exec(db, table, NULL, NULL, &create_err);
  if (create_stat != SQLITE_OK) {
    char *err_msg = strdup(create_err);
    sqlite3_free(create_err);
    return (Result){.type = DB_ERR, .value = err_msg};
  } else {
    return (Result){.type = SUCCESS, .value = ""};
  }
}
