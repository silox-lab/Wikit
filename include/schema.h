/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#ifndef SCHEMAS
#define SCHEMAS
#include <stdint.h>
#include <time.h>

typedef struct ScopeFile {
  int64_t id;
  char *name;
  char *extension;
  time_t created_on;
  int64_t scope_id;
} ScopeFile;

typedef struct Scope {
  int64_t id;
  char *name;
  time_t created_on;
  int64_t wiki_id;
  int64_t parent_scope_id;
} Scope;

typedef struct Wiki {
  int64_t id;
  char *name;
  char *description;
  time_t created_on;
} Wiki;

#define DB_SCOPE_FILE                                                          \
  "CREATE TABLE IF NOT EXISTS ScopeFile ("                                     \
  "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,"                                                        \
  "name varchar(250) NOT NULL,"                                                \
  "extension varchar(250),"                                                    \
  "created_on integer NOT NULL,"                                                   \
  "scope_id integer NOT NULL,"                                                              \
  "FOREIGN KEY(scope_id) REFERENCES Scope(id) ON DELETE CASCADE"                                  \
  ");"

#define DB_SCOPE                                                               \
  "CREATE TABLE IF NOT EXISTS Scope ("                                         \
  "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,"                                                        \
  "name varchar(250) NOT NULL,"                                                \
  "created_on integer NOT NULL,"                                                            \
  "wiki_id integer NOT NULL,"                                                               \
  "parent_scope_id integer,"                                                       \
  "FOREIGN KEY(wiki_id) REFERENCES Wiki(id) ON DELETE CASCADE,"                                   \
  "FOREIGN KEY(parent_scope_id) REFERENCES Scope(id) ON DELETE CASCADE"                           \
  ");"

#define DB_WIKI                                                                \
  "CREATE TABLE IF NOT EXISTS Wiki ("                                          \
  "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,"                                                        \
  "name varchar(250) UNIQUE NOT NULL,"                                         \
  "description text NOT NULL,"                                                  \
  "created_on integer NOT NULL"                                                    \
  ");"

#endif