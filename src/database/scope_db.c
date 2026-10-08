/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#include "../../include/errors.h"
#include "../../include/globals.h"
#include "../../include/scopefiles.h"
#include "../../include/database/scopefile_db.h"
#include "../../include/scopes.h"
#include "../../include/schema.h"
#include <stdint.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>

Result get_scope_db(int64_t id, int64_t wiki_id, int64_t parent_scope_id, char *name) {

  sqlite3_stmt *get_scope_stmt;
  sqlite3 *db = GET_W_DATABASE();
  
  int arr_length = 0;
  int arr_capacity = 10;
  Scope **scopes_arr = malloc(sizeof(Scope *) * arr_capacity);
  
  if (scopes_arr == NULL) {
    return (Result){.type = SIMPLE_ERR, .value = "memory allocation failed"};
  }

  char sql[512] = "SELECT * FROM Scope WHERE 1=1";
  char where_clause[512] = "";
  
  if (id != 0) {
    strcat(where_clause, " AND id = ? ");
  }
  if (wiki_id != 0) {
    strcat(where_clause, " AND wiki_id = ? ");
  }
  if (parent_scope_id != 0) {
    strcat(where_clause, " AND parent_scope_id = ? ");
  }
  if (name != NULL) {
    strcat(where_clause, " AND name = ? ");
  }

  strcat(where_clause, ";");
  strcat(sql, where_clause);

  int prepare_stat = sqlite3_prepare_v2(db, sql, -1, &get_scope_stmt, NULL);
  if (prepare_stat != SQLITE_OK) {
    free(scopes_arr);
    return (Result){.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  int bind_index = 1;
  if (id != 0) {
    sqlite3_bind_int(get_scope_stmt, bind_index++, id);
  }
  if (wiki_id != 0) {
    sqlite3_bind_int(get_scope_stmt, bind_index++, wiki_id);
  }
  if (parent_scope_id != 0) {
    sqlite3_bind_int(get_scope_stmt, bind_index++, parent_scope_id);
  }
  if (name != NULL) {
    sqlite3_bind_text(get_scope_stmt, bind_index++, name, -1, SQLITE_STATIC);
  }

  int step_result;
  while ((step_result = sqlite3_step(get_scope_stmt)) == SQLITE_ROW) {
    Scope *scope = malloc(sizeof(Scope));

    if (scope == NULL) {
      int finalize_stat = sqlite3_finalize(get_scope_stmt);
      if (finalize_stat != SQLITE_OK) {
        return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
      }
      for (int i = 0; i != arr_length; i++) {
        free(scopes_arr[i]);
      }
      free(scopes_arr);
      return (Result){.type = SIMPLE_ERR, .value = "memory allocation failed"};
    }

    if (arr_length >= arr_capacity) {
      arr_capacity *= 2;
      Scope **temp = realloc(scopes_arr, sizeof(Scope *) * arr_capacity);
      if (temp == NULL) {
        free(scope);
        sqlite3_finalize(get_scope_stmt);
        for (int i = 0; i != arr_length; i++) {
          free(scopes_arr[i]->name);
          free(scopes_arr[i]);
        }
        free(scopes_arr);
        return (Result){.type = SIMPLE_ERR, .value = "memory reallocation failed"};
      }
      scopes_arr = temp;
    }

    scope->id = sqlite3_column_int64(get_scope_stmt, 0);
    const char *name_text = (const char *)sqlite3_column_text(get_scope_stmt, 1);
    
    if (name_text) scope->name = strdup(name_text);
    
    scope->created_on = sqlite3_column_int(get_scope_stmt, 2);
    scope->wiki_id = sqlite3_column_int64(get_scope_stmt, 3);
    
    int parent_id_type = sqlite3_column_type(get_scope_stmt, 4);
    if (parent_id_type != SQLITE_NULL) {
      scope->parent_scope_id = sqlite3_column_int64(get_scope_stmt, 4);
    } else {
      scope->parent_scope_id = 0;
    }

    scopes_arr[arr_length] = scope;
    arr_length++;
  }

  if (arr_length == 0) {
    free(scopes_arr);
    return (Result) { .type = SUCCESS, .value = NULL };
  }

  if (step_result != SQLITE_DONE) {
    char *err_msg = strdup(sqlite3_errmsg(db));
    int finalize_stat = sqlite3_finalize(get_scope_stmt);
    if (finalize_stat != SQLITE_OK) {
      return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
    }
    for (int i = 0; i != arr_length; i++) {
      free(scopes_arr[i]->name);
      free(scopes_arr[i]);
    }
    free(scopes_arr);
    return (Result){.type = DB_ERR, .value = err_msg};
  }

  scopes_arr[arr_length] = NULL;
  arr_length++;

  int finalize_stat = sqlite3_finalize(get_scope_stmt);
  if (finalize_stat != SQLITE_OK) {
    for (int i = 0; i != arr_length - 1; i++) {
      free(scopes_arr[i]->name);
      free(scopes_arr[i]);
    }
    free(scopes_arr);
    return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  return (Result){.type = SUCCESS, .value = scopes_arr};
}

Result create_scope_db(Scope *new_scope, Wiki *wiki) {

  sqlite3 *db = GET_W_DATABASE();

  sqlite3_stmt *create_stmt;
  const char *sql = "INSERT INTO Scope(name, created_on, parent_scope_id, wiki_id) VALUES (?, ?, ?, ?);";
  Result scope_exist = get_scope_db(0, new_scope->wiki_id, new_scope->parent_scope_id, new_scope->name);
  if (scope_exist.type != SUCCESS) {
    return (Result) {
        .type = scope_exist.type,
        .value = scope_exist.value
    };
  }

  Scope **scope_exist_arr = scope_exist.value;

  for (int i=0; scope_exist_arr != NULL && scope_exist_arr[i] != NULL ; i++) {
    if (strcmp(scope_exist_arr[i]->name, new_scope->name) == 0) {
      for (int i2=0; scope_exist_arr[i2] != NULL ; i2++) {
        free(scope_exist_arr[i2]->name);
        free(scope_exist_arr[i2]);
      }
      free(scope_exist_arr);
      return (Result) {
        .type = SIMPLE_ERR,
        .value = "scope with same name exist"
      };
    }
  }

  int prepare_stat = sqlite3_prepare_v2(db, sql, -1, &create_stmt, NULL);
  if (prepare_stat != SQLITE_OK) {
    for (int i=0; scope_exist_arr != NULL && scope_exist_arr[i] != NULL ; i++) {
      free(scope_exist_arr[i]->name);
      free(scope_exist_arr[i]);
    }
    free(scope_exist_arr);
    return (Result){.type = DB_ERR,
                    .value = strdup(sqlite3_errmsg(db))};
  }

  sqlite3_bind_text(create_stmt, 1, new_scope->name, -1, SQLITE_STATIC);
  sqlite3_bind_int64(create_stmt, 2, (int64_t)time(NULL));
  if (new_scope->parent_scope_id == 0) {
    sqlite3_bind_null(create_stmt, 3);  
  }
  if (new_scope->parent_scope_id != 0) {
    sqlite3_bind_int64(create_stmt,3, new_scope->parent_scope_id);
  }
  sqlite3_bind_int64(create_stmt, 4, wiki->id);

  int step_stat = sqlite3_step(create_stmt);
  if (step_stat != SQLITE_DONE) {
    for (int i=0; scope_exist_arr != NULL && scope_exist_arr[i] != NULL ; i++) {
      free(scope_exist_arr[i]->name);
      free(scope_exist_arr[i]);
    }
    free(scope_exist_arr);
    char *err_msg = strdup(sqlite3_errmsg(db));
    int finalize_stat = sqlite3_finalize(create_stmt);
    if (finalize_stat != SQLITE_OK) {
      return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
    }
    return (Result){.type = DB_ERR, .value = err_msg};
  }

  for (int i=0; scope_exist_arr != NULL && scope_exist_arr[i] != NULL ; i++) {
    free(scope_exist_arr[i]->name);
    free(scope_exist_arr[i]);
  }
  free(scope_exist_arr);
  int finalize_stat = sqlite3_finalize(create_stmt);
  if (finalize_stat != SQLITE_OK) {
    return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  Result get_created_scope = get_scope_db(0, new_scope->wiki_id, new_scope->parent_scope_id, new_scope->name);
  if (get_created_scope.type != SUCCESS) {
    return (Result) {
        .type = get_created_scope.type,
        .value = get_created_scope.value
    };
  }

  Scope *created_scope = ((Scope **)get_created_scope.value)[0];

  Result make_scope_dir = make_scope(*created_scope);
  if (make_scope_dir.type != SUCCESS) {
    return (Result) { .type = make_scope_dir.type, .value = make_scope_dir.value };
  }

  ScopeFile main_sf = { .scope_id = created_scope->id, .extension = "txt", .name = "main", .created_on = time(NULL) };

  Result make_main_sf = make_scopefile(main_sf);
  if (make_main_sf.type != SUCCESS) {
    return (Result) { .type = make_main_sf.type, .value = make_main_sf.value };
  }

  Result make_main_sf_db = create_scopefile_db(&main_sf);
  if (make_main_sf_db.type != SUCCESS) {
    return (Result) { .type = make_main_sf_db.type, .value = make_main_sf_db.value };
  }

  
  return (Result){.type = SUCCESS, .value = ""};
}

Result delete_scope_db(int64_t id) {
  sqlite3 *db = GET_W_DATABASE();
  sqlite3_stmt *del_scope_stmt;

  int prepare_stat = sqlite3_prepare_v2(db, "DELETE FROM Scope WHERE id = ?;",
     -1, &del_scope_stmt, NULL);
  
  if (prepare_stat != SQLITE_OK) {
    return (Result){
      .type = DB_ERR,
      .value = strdup(sqlite3_errmsg(db))
    };
  }

  sqlite3_bind_int64(del_scope_stmt, 1, id);
  int del_scope_step = sqlite3_step(del_scope_stmt);
  if (del_scope_step != SQLITE_DONE) {
    int finalize_stat = sqlite3_finalize(del_scope_stmt);
    if (finalize_stat != SQLITE_OK) {
      return (Result){ .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
    }

    return (Result){ .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }

  int finalize_stat = sqlite3_finalize(del_scope_stmt);
  if (finalize_stat != SQLITE_OK) {
    return (Result){ .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }

  return (Result){ .type = SUCCESS, .value = "" };
}

Result update_scope_db(int64_t id, Scope s) {

  Result update_s = update_scope(id, s.name);
  if (update_s.type != SUCCESS) {
    return (Result) { .type = update_s.type, .value = update_s.value };
  }
  
  sqlite3 *db = GET_W_DATABASE();
  sqlite3_stmt *update_scope_stmt;

  char sql[512] = "UPDATE Scope SET";
  char update_clause[512] = "";
  char where_clause[256] = " WHERE id = ?;";

  Result scope_exists = get_scope_db(id, 0, 0, 0);
  if (scope_exists.type != SUCCESS) {
    return (Result) { .type = scope_exists.type, .value = scope_exists.value };
  }

  if (scope_exists.type == SUCCESS && scope_exists.value == NULL) {
    return (Result) { .type = SIMPLE_ERR, .value = "Scope not found." };
  }

  for (int i=0; ((Scope **)scope_exists.value)[i] != NULL; i++) {
    free(((Scope **)scope_exists.value)[i]->name);
  }
  free(scope_exists.value);

  int to_update = 0;

  if (s.name != NULL) to_update++;
  
  int added_to = 0;

  if (s.name != NULL) {
    added_to++;
    strcat(update_clause, " name = ?");
    if (added_to != to_update && to_update != 1) {
      strcat(update_clause, ",");
    }
  }

  strcat(sql, update_clause);
  strcat(sql, where_clause);

  int prepare_stat = sqlite3_prepare_v2(db, sql, -1, &update_scope_stmt, NULL);
  if (prepare_stat != SQLITE_OK) {
    return (Result) { .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }

  int bind_index = 1;
  
  if (s.name != NULL) {
    sqlite3_bind_text(update_scope_stmt, bind_index++, s.name, -1, SQLITE_STATIC);
  }

  sqlite3_bind_int64(update_scope_stmt, bind_index++, id);
  
  int step_stat = sqlite3_step(update_scope_stmt);

  if (step_stat != SQLITE_DONE) {
    if (sqlite3_finalize(update_scope_stmt) != SQLITE_OK) {
      return (Result) { .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
    }
    return (Result) { .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }

  int finalize_stat = sqlite3_finalize(update_scope_stmt);
  if (finalize_stat != SQLITE_OK) {
    return (Result){ .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }

  return (Result) { .type = SUCCESS, .value = "" };
}