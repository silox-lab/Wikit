/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#include "../../include/errors.h"
#include "../../include/globals.h"
#include "../../include/schema.h"
#include "../../include/wikis.h"
#include <stdint.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>

Result get_wiki_db(char *name, int64_t id) {

  sqlite3_stmt *get_wiki_stmt;
  sqlite3 *db = GET_W_DATABASE();
  
  int arr_length = 0;
  int arr_capacity = 10;
  Wiki **wikis_arr = malloc(sizeof(Wiki *) * arr_capacity);
  
  if (wikis_arr == NULL) {
    return (Result){.type = SIMPLE_ERR, .value = "memory allocation failed"};
  }

  char sql[512] = "SELECT * FROM Wiki WHERE 1=1 ";
  char where_clause[512] = "";
  
  int bind_index = 1;
  if (name != NULL) {
    strcat(where_clause, " AND name = ?");
  }
  if (id != 0) {
    strcat(where_clause, " AND id = ?");
  }

  strcat(where_clause, ";");
  strcat(sql, where_clause);

  int prepare_stat = sqlite3_prepare_v2(db, sql, -1, &get_wiki_stmt, NULL);
  if (prepare_stat != SQLITE_OK) {
    free(wikis_arr);
    return (Result){.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  if (name != NULL) {
    sqlite3_bind_text(get_wiki_stmt, bind_index++, name, -1, SQLITE_STATIC);
  }
  if (id != 0) {
    sqlite3_bind_int(get_wiki_stmt, bind_index++, id);
  }

  int step_result;
  while ((step_result = sqlite3_step(get_wiki_stmt)) == SQLITE_ROW) {
    Wiki *wiki = malloc(sizeof(Wiki));

    if (wiki == NULL) {
      int finalize_stat = sqlite3_finalize(get_wiki_stmt);
      if (finalize_stat != SQLITE_OK) {
        for (int i = 0; i < arr_length; i++) {
          free(wikis_arr[i]);
        }
        free(wikis_arr);
        return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
      }
      for (int i = 0; i < arr_length; i++) {
        free(wikis_arr[i]);
      }
      free(wikis_arr);
      return (Result){.type = SIMPLE_ERR, .value = "memory allocation failed"};
    }

    if (arr_length >= arr_capacity) {
      arr_capacity *= 2;
      Wiki **temp = realloc(wikis_arr, sizeof(Wiki *) * arr_capacity);
      if (temp == NULL) {
        free(wiki);
        sqlite3_finalize(get_wiki_stmt);
        for (int i = 0; i < arr_length; i++) {
          free(wikis_arr[i]->name);
          free(wikis_arr[i]->description);
          free(wikis_arr[i]);
        }
        free(wikis_arr);
        return (Result){.type = SIMPLE_ERR, .value = "memory reallocation failed"};
      }
      wikis_arr = temp;
    }

    wiki->id = sqlite3_column_int64(get_wiki_stmt, 0);
    const char *name_text = (const char *)sqlite3_column_text(get_wiki_stmt, 1);
    const char *description_text = (const char *)sqlite3_column_text(get_wiki_stmt, 2);
    
    if (name_text) wiki->name = strdup(name_text);
    if (description_text) wiki->description = strdup(description_text);
    
    wiki->created_on = sqlite3_column_int(get_wiki_stmt, 3);

    wikis_arr[arr_length] = wiki;
    arr_length++;
  }

  if (arr_length == 0) {
    sqlite3_finalize(get_wiki_stmt);
    free(wikis_arr);

    return (Result){
        .type = SUCCESS,
        .value = NULL
  };
  }

  wikis_arr[arr_length] = NULL;
  arr_length++;

  if (step_result != SQLITE_DONE) {
    char *err_msg = strdup(sqlite3_errmsg(db));
    int finalize_stat = sqlite3_finalize(get_wiki_stmt);
    if (finalize_stat != SQLITE_OK) {
      for (int i = 0; i != arr_length - 1; i++) {
        free(wikis_arr[i]->name);
        free(wikis_arr[i]->description);
        free(wikis_arr[i]);
      }
      free(wikis_arr);
      return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
    }
    for (int i = 0; i != arr_length - 1; i++) {
      free(wikis_arr[i]->name);
      free(wikis_arr[i]->description);
      free(wikis_arr[i]);
    }
    free(wikis_arr);
    return (Result){.type = DB_ERR, .value = err_msg};
  }

  int finalize_stat = sqlite3_finalize(get_wiki_stmt);
  if (finalize_stat != SQLITE_OK) {
    for (int i = 0; i != arr_length - 1; i++) {
      free(wikis_arr[i]->name);
      free(wikis_arr[i]->description);
      free(wikis_arr[i]);
    }
    free(wikis_arr);
    return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  return (Result){.type = SUCCESS, .value = wikis_arr};
}

Result create_wiki_db(Wiki *wiki) {
  sqlite3 *db = GET_W_DATABASE();
  sqlite3_stmt *create_stmt;
  const char *sql = "INSERT INTO Wiki(name, description, created_on) VALUES (?, ?, ?);";

  Result wiki_exist = get_wiki_db(wiki->name, 0);
  if (wiki_exist.type != SUCCESS) {

    return (Result){.type = wiki_exist.type,
                    .value = wiki_exist.value };
  }

  Wiki **wiki_exist_arr = wiki_exist.value;

  if (wiki_exist_arr != NULL) {
    return (Result) {
      .type = SIMPLE_ERR,
      .value = "wiki with same name exist"
    };
  } 

  int prepare_stat = sqlite3_prepare_v2(db, sql, -1, &create_stmt, NULL);
  if (prepare_stat != SQLITE_OK) {
    for (int i=0; wiki_exist_arr != NULL && wiki_exist_arr[i] != NULL ; i++) {
      free(wiki_exist_arr[i]->name);
      free(wiki_exist_arr[i]->description);
      free(wiki_exist_arr[i]);
    }
    free(wiki_exist_arr);
    return (Result){.type = DB_ERR,
                    .value = strdup(sqlite3_errmsg(db))};
  }
  
  sqlite3_bind_text(create_stmt, 1, wiki->name, -1, SQLITE_STATIC);
  sqlite3_bind_text(create_stmt, 2, wiki->description, -1, SQLITE_STATIC);
  sqlite3_bind_int64(create_stmt, 3, (int64_t)time(NULL));

  int step_stat = sqlite3_step(create_stmt);
  if (step_stat != SQLITE_DONE) {
    char *err_msg = strdup(sqlite3_errmsg(db));
    sqlite3_finalize(create_stmt);
    for (int i=0; wiki_exist_arr != NULL && wiki_exist_arr[i] != NULL ; i++) {
      free(wiki_exist_arr[i]->name);
      free(wiki_exist_arr[i]->description);
      free(wiki_exist_arr[i]);
    }
    free(wiki_exist_arr);
    return (Result){.type = DB_ERR, .value = err_msg};
  }

  Result make_wiki_stat = make_wiki(wiki->name);
  if (make_wiki_stat.type != SUCCESS) {
    for (int i=0; wiki_exist_arr != NULL && wiki_exist_arr[i] != NULL ; i++) {
      free(wiki_exist_arr[i]->name);
      free(wiki_exist_arr[i]->description);
      free(wiki_exist_arr[i]);
    }
    free(wiki_exist_arr);
    int finalize_stat = sqlite3_finalize(create_stmt);
    if (finalize_stat != SQLITE_OK) {
      return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
    }

    return (Result) {.type = SIMPLE_ERR, .value = make_wiki_stat.value};
  }

  int finalize_stat = sqlite3_finalize(create_stmt);
  if (finalize_stat != SQLITE_OK) {
    for (int i=0; wiki_exist_arr != NULL && wiki_exist_arr[i] != NULL ; i++) {
      free(wiki_exist_arr[i]->name);
      free(wiki_exist_arr[i]->description);
      free(wiki_exist_arr[i]);
    }
    free(wiki_exist_arr);
    return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }
  for (int i=0; wiki_exist_arr != NULL && wiki_exist_arr[i] != NULL ; i++) {
    free(wiki_exist_arr[i]->name);
    free(wiki_exist_arr[i]->description);
    free(wiki_exist_arr[i]);
  }

  free(wiki_exist_arr);

  return (Result){.type = SUCCESS, .value = ""};
}

Result get_all_wikis() {
  char sql[256] = "SELECT * FROM Wiki;";
  sqlite3_stmt *get_all_wikis_stmt;
  sqlite3 *db = GET_W_DATABASE();
  int capacity = 10;
  int length = 0;
  Wiki **wikis_list = malloc(sizeof(Wiki *) * capacity);
  if (wikis_list == NULL) {
    return (Result) { .type = SIMPLE_ERR, .value = "memory allocation failed." };
  }
  int prepare_stat = sqlite3_prepare_v2(db, sql, -1, &get_all_wikis_stmt, NULL);
  if (prepare_stat != SQLITE_OK) {
    free(wikis_list);
    char *err_msg = strdup(sqlite3_errmsg(db));
    return (Result){ .type = DB_ERR, .value = err_msg  };
  }
  
  while (sqlite3_step(get_all_wikis_stmt) == SQLITE_ROW) {
    Wiki *wiki = malloc(sizeof(Wiki));
    if (wiki == NULL) {
      for (int i=0; i < length; i++) {
        free(wikis_list[i]->name);
        free(wikis_list[i]->description);
        free(wikis_list[i]);
      }
      free(wikis_list);
      int finalize_stat = sqlite3_finalize(get_all_wikis_stmt);
      if (finalize_stat != SQLITE_OK) {
        return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
      }
      return (Result) {.type = SIMPLE_ERR, .value = "memory allocation failed."};
    }
  
    wiki->id = sqlite3_column_int64(get_all_wikis_stmt, 0);
    const char *name = (const char *)sqlite3_column_text(get_all_wikis_stmt, 1);
    const char *description = (const char *)sqlite3_column_text(get_all_wikis_stmt, 2);
    wiki->created_on = sqlite3_column_int(get_all_wikis_stmt, 3);

    wiki->name = strdup(name);
    wiki->description = strdup(description);

    if (length >= capacity) {
      capacity *= 2;
      Wiki **tmp_wikis_list = realloc(wikis_list, capacity);
      if (tmp_wikis_list == NULL) {
        free(wiki->name);
        free(wiki->description);
        free(wiki);
        for (int i=0; i < length; i++) {
          free(wikis_list[i]->name);
          free(wikis_list[i]->description);
          free(wikis_list[i]);
        }
        free(wikis_list);
        int finalize_stat = sqlite3_finalize(get_all_wikis_stmt);
        if (finalize_stat != SQLITE_OK) {
          return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
        } 
        return (Result) { .type = SIMPLE_ERR, .value = "memory reallocation failed."};
      }
      wikis_list = tmp_wikis_list;
    }

    wikis_list[length] = wiki;
    length++;
  }

  wikis_list[length] = NULL;

  int finalize_stat = sqlite3_finalize(get_all_wikis_stmt);
  if (finalize_stat != SQLITE_OK) {
    return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  return (Result) { .type = SUCCESS, .value = wikis_list };
}

Result get_wiki_scopes_db(int64_t wiki_id) {
  
  sqlite3 *db = GET_W_DATABASE();
  char sql[256] = "SELECT * FROM Scope WHERE wiki_id = ?;";
  sqlite3_stmt *get_wiki_scopes_stmt;

  int capacity = 10;
  int length = 0;
  Scope **scopes_list = malloc(sizeof(Scope *) * capacity);
  if (scopes_list == NULL) {
    int finalize_stat = sqlite3_finalize(get_wiki_scopes_stmt);
    if (finalize_stat != SQLITE_OK) {
      return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
    }
    return (Result) {.type = SIMPLE_ERR, .value = "memory allocation failed."};
  }

  int prepare_stat = sqlite3_prepare_v2(db, sql, -1, &get_wiki_scopes_stmt, NULL);
  if (prepare_stat != SQLITE_OK) {
    free(scopes_list);
    return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  sqlite3_bind_int64(get_wiki_scopes_stmt, 1, wiki_id);

  while (sqlite3_step(get_wiki_scopes_stmt) == SQLITE_ROW) {
    Scope *scope = malloc(sizeof(Scope));
    if (scope == NULL) {
      int finalize_stat = sqlite3_finalize(get_wiki_scopes_stmt);
      if (finalize_stat != SQLITE_OK) {
        for (int i=0; i < length; i++) {
          free(scopes_list[i]->name);
          free(scopes_list[i]);
        }
        free(scopes_list);
        return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
      }
      for (int i=0; i < length; i++) {
        free(scopes_list[i]->name);
        free(scopes_list[i]);
      }
      free(scopes_list);
      return (Result) {.type = SIMPLE_ERR, .value = "memory allocation failed."};
    }

    scope->id = sqlite3_column_int64(get_wiki_scopes_stmt, 0);
    scope->name = strdup((char *)sqlite3_column_text(get_wiki_scopes_stmt, 1));
    scope->created_on = sqlite3_column_int(get_wiki_scopes_stmt, 3);
    scope->wiki_id = sqlite3_column_int64(get_wiki_scopes_stmt, 4);
    scope->parent_scope_id = sqlite3_column_int64(get_wiki_scopes_stmt, 5);
    
    if (capacity == length) {
      capacity *= 2;
      Scope **tmp = realloc(scopes_list, capacity);
      if (tmp == NULL) {
        free(scope->name);
        free(scope);
        for (int i=0; i < length; i++) {
          free(scopes_list[i]->name);
          free(scopes_list[i]);
        }
        
        free(scopes_list);
        int finalize_stat = sqlite3_finalize(get_wiki_scopes_stmt);
        if (finalize_stat != SQLITE_OK) {
          return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
        }

        return (Result) { .type = SIMPLE_ERR, .value = "Memory reallocation failed."};
      }
      scopes_list = tmp;
    }

    scopes_list[length] = scope;
    length++;
  }

  scopes_list[length] = NULL;

  int finalize_stat = sqlite3_finalize(get_wiki_scopes_stmt);
  if (finalize_stat != SQLITE_OK) {
    for (int i=0; i < length; i++) {
      free(scopes_list[i]->name);
      free(scopes_list[i]);
    }
    free(scopes_list);

    return (Result) {.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  return (Result) {.type = SUCCESS, .value = scopes_list};
}

Result delete_wiki_db(int64_t id) {

  sqlite3 *db = GET_W_DATABASE();
  sqlite3_stmt *delete_wiki_stmt;

  Result wiki_exist = get_wiki_db(NULL, id);
  Wiki ** wiki_exist_arr = wiki_exist.value;

  if (wiki_exist_arr == NULL) {
    return (Result) { .type = SIMPLE_ERR, .value = "Wiki not found." };
  }

  int del_wiki_prepare = sqlite3_prepare_v2(db, 
    "DELETE FROM Wiki WHERE id = ?;", -1, &delete_wiki_stmt, NULL);
  
  if (del_wiki_prepare != SQLITE_OK) {
    return (Result){ .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }
  
  sqlite3_bind_int64(delete_wiki_stmt, 1, id);
  
  int step_del_wiki = sqlite3_step(delete_wiki_stmt);

  if (step_del_wiki != SQLITE_DONE) {
    int finalize_stat = sqlite3_finalize(delete_wiki_stmt);
    if (finalize_stat != SQLITE_OK) {
      return (Result){ .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
    }

    return (Result){ .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }

  int finalize_stat = sqlite3_finalize(delete_wiki_stmt);
  if (finalize_stat != SQLITE_OK) {
    return (Result){ .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }

  Result del_wiki_dir = delete_wiki(wiki_exist_arr[0]->name);
  if (del_wiki_dir.type != SUCCESS) {
    return (Result) { .type = del_wiki_dir.type, .value = del_wiki_dir.value };
  }
  
  return (Result) { .type = SUCCESS, .value = "" };
}

Result update_wiki_db(char *name, Wiki w) {

  Result update_w = update_wiki(name, w.name);
  if (update_w.type != SUCCESS) {
    return (Result) { .type = update_w.type, .value = update_w.value };
  }

  sqlite3 *db = GET_W_DATABASE();
  sqlite3_stmt *update_wiki_stmt;

  char sql[512] = "UPDATE Wiki SET";
  char update_clause[512] = "";
  char where_clause[256] = " WHERE name = ?;";

  Result wiki_exists = get_wiki_db(name, 0);
  if (wiki_exists.type != SUCCESS) {
    return (Result) { .type = wiki_exists.type, .value = wiki_exists.value };
  }

  if (wiki_exists.type == SUCCESS && wiki_exists.value == NULL) {
    return (Result) { .type = SIMPLE_ERR, .value = "Wiki not found." };
  }

  for (int i=0; ((Wiki **)wiki_exists.value)[i] != NULL; i++) {
    free(((Wiki **)wiki_exists.value)[i]->description);
    free(((Wiki **)wiki_exists.value)[i]->name);
  }
  free(wiki_exists.value);

  int to_update = 0;

  if (w.description != NULL) to_update++;
  if (w.name != NULL) to_update++;
  
  int added_to = 0;

  if (w.description != NULL) {
    added_to++;
    strcat(update_clause, " description = ?");
    if (added_to != to_update && to_update != 1) {
      strcat(update_clause, ",");
    }
  }

  if (w.name != NULL) {
    added_to++;
    strcat(update_clause, " name = ?");
    if (added_to != to_update && to_update != 1) {
      strcat(update_clause, ",");
    }
  }

  strcat(sql, update_clause);
  strcat(sql, where_clause);

  int prepare_stat = sqlite3_prepare_v2(db, sql, -1, &update_wiki_stmt, NULL);
  if (prepare_stat != SQLITE_OK) {
    return (Result) { .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }

  int bind_index = 1;
  if (w.description != NULL) {
    sqlite3_bind_text(update_wiki_stmt, bind_index++, w.description, -1, SQLITE_STATIC);
  }

  
  if (w.name != NULL) {
    sqlite3_bind_text(update_wiki_stmt, bind_index++, w.name, -1, SQLITE_STATIC);
  }

  sqlite3_bind_text(update_wiki_stmt, bind_index++, name, -1, SQLITE_STATIC);
  
  int step_stat = sqlite3_step(update_wiki_stmt);

  if (step_stat != SQLITE_DONE) {
    if (sqlite3_finalize(update_wiki_stmt) != SQLITE_OK) {
      return (Result) { .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
    }
    return (Result) { .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }

  int finalize_stat = sqlite3_finalize(update_wiki_stmt);
  if (finalize_stat != SQLITE_OK) {
    return (Result){ .type = DB_ERR, .value = strdup(sqlite3_errmsg(db)) };
  }

  return (Result) { .type = SUCCESS, .value = "" };
}
