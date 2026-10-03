/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#include "../../include/errors.h"
#include "../../include/globals.h"
#include "../../include/schema.h"
#include "../../include/scopefiles.h"
#include <sqlite3.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Result get_scopefile_db(char *scopefile_name, int64_t scope_id)
{
    sqlite3 *db = GET_W_DATABASE();
    sqlite3_stmt *get_scopefile_stmt;

    int arr_length = 0;
    int arr_capacity = 10;

    ScopeFile **scopefiles_arr =
        malloc(sizeof(ScopeFile *) * arr_capacity);

    if (scopefiles_arr == NULL) {
        return (Result){
            .type = SIMPLE_ERR,
            .value = "memory allocation failed."
        };
    }

    char sql[512] = "SELECT * FROM ScopeFile WHERE 1=1 ";
    char where_clause[512] = "";

    if (scope_id != 0) {
        strcat(where_clause, "AND scope_id = ? ");
    }

    if (scopefile_name != NULL) {
        strcat(where_clause, "AND name = ? ");
    }

    strcat(where_clause, ";");
    strcat(sql, where_clause);

    int prepare_stat =
        sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &get_scopefile_stmt,
            NULL
        );

    if (prepare_stat != SQLITE_OK) {
        free(scopefiles_arr);

        return (Result){
            .type = DB_ERR,
            .value = strdup(sqlite3_errmsg(db))
        };
    }

    int bind_index = 1;

    if (scope_id != 0) {
        sqlite3_bind_int64(
            get_scopefile_stmt,
            bind_index++,
            scope_id
        );
    }

    if (scopefile_name != NULL) {
        sqlite3_bind_text(
            get_scopefile_stmt,
            bind_index++,
            scopefile_name,
            -1,
            SQLITE_STATIC
        );
    }

    int get_scopefile_stat;

    while ((get_scopefile_stat =
                sqlite3_step(get_scopefile_stmt)) == SQLITE_ROW) {

        ScopeFile *scopefile =
            malloc(sizeof(ScopeFile));

        if (scopefile == NULL) {
            sqlite3_finalize(get_scopefile_stmt);

            for (int i = 0; i < arr_length; i++) {
                free(scopefiles_arr[i]->name);
                free(scopefiles_arr[i]->extension);
                free(scopefiles_arr[i]);
            }

            free(scopefiles_arr);

            return (Result){
                .type = SIMPLE_ERR,
                .value = "memory allocation failed."
            };
        }

        scopefile->id =
            sqlite3_column_int64(get_scopefile_stmt, 0);

        scopefile->name =
            strdup((const char *)
                sqlite3_column_text(get_scopefile_stmt, 1));

        scopefile->extension =
            strdup((const char *)
                sqlite3_column_text(get_scopefile_stmt, 2));

        scopefile->created_on =
            sqlite3_column_int(get_scopefile_stmt, 3);

        scopefile->scope_id =
            sqlite3_column_int64(get_scopefile_stmt, 4);

        if (arr_length >= arr_capacity - 1) {

            arr_capacity *= 2;

            ScopeFile **tmp =
                realloc(
                    scopefiles_arr,
                    sizeof(ScopeFile *) * arr_capacity
                );

            if (tmp == NULL) {
                free(scopefile);

                sqlite3_finalize(get_scopefile_stmt);

                for (int i = 0; i < arr_length; i++) {
                    free(scopefiles_arr[i]->name);
                    free(scopefiles_arr[i]->extension);
                    free(scopefiles_arr[i]);
                }

                free(scopefiles_arr);

                return (Result){
                    .type = SIMPLE_ERR,
                    .value = "memory allocation failed."
                };
            }

            scopefiles_arr = tmp;
        }

        scopefiles_arr[arr_length++] = scopefile;
    }

    if (get_scopefile_stat != SQLITE_DONE) {

        char *err_msg =
            strdup(sqlite3_errmsg(db));

        sqlite3_finalize(get_scopefile_stmt);

        for (int i = 0; i < arr_length; i++) {
            free(scopefiles_arr[i]->name);
            free(scopefiles_arr[i]->extension);
            free(scopefiles_arr[i]);
        }

        free(scopefiles_arr);

        return (Result){
            .type = DB_ERR,
            .value = err_msg
        };
    }

    sqlite3_finalize(get_scopefile_stmt);

    if (arr_length == 0) {
        free(scopefiles_arr);

        return (Result){
            .type = SUCCESS,
            .value = NULL
        };
    }

    scopefiles_arr[arr_length] = NULL;

    return (Result){
        .type = SUCCESS,
        .value = scopefiles_arr
    };
}

Result create_scopefile_db(ScopeFile *scopefile) {

  sqlite3 *db = GET_W_DATABASE();

  sqlite3_stmt *create_stmt;
  const char *sql = "INSERT INTO ScopeFile(name, extension, created_on, "
                    "scope_id) VALUES (?, ?, ?, ?);";

  int prepare_stat = sqlite3_prepare_v2(db, sql, -1, &create_stmt, NULL);
  if (prepare_stat != SQLITE_OK) {
    return (Result){.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }
  Result scopefile_exist_result =
      get_scopefile_db(scopefile->name, scopefile->scope_id);
  if (scopefile_exist_result.type == SUCCESS &&
      scopefile_exist_result.value != NULL) {
    if (strcmp(((ScopeFile **)(scopefile_exist_result.value))[0]->name,
               scopefile->name) == 0) {
      return (Result){.type = SIMPLE_ERR,
                      .value = "scopefile with same name exist"};
    }
  }

  sqlite3_bind_text(create_stmt, 1, scopefile->name, -1, SQLITE_STATIC);
  sqlite3_bind_text(create_stmt, 2, scopefile->extension, -1, SQLITE_STATIC);
  sqlite3_bind_int(create_stmt, 3, scopefile->created_on);
  sqlite3_bind_int64(create_stmt, 4, scopefile->scope_id);

  int step_stat = sqlite3_step(create_stmt);
  if (step_stat != SQLITE_DONE) {
    char *err_msg = strdup(sqlite3_errmsg(db));
    int finalize_stat = sqlite3_finalize(create_stmt);
    if (finalize_stat != SQLITE_OK) {
      return (Result){.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
    }
    return (Result){.type = DB_ERR, .value = err_msg};
  }

  int finalize_stat = sqlite3_finalize(create_stmt);
  if (finalize_stat != SQLITE_OK) {
    return (Result){.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  Result make_scopefile_dir = make_scopefile(*scopefile);
  if (make_scopefile_dir.type != SUCCESS) {
    return (Result) { .type = make_scopefile_dir.type, .value = make_scopefile_dir.value };
  }
  
  return (Result){.type = SUCCESS, .value = ""};
}

Result delete_scopefile_db(int64_t id) {

  sqlite3 *db = GET_W_DATABASE();
  sqlite3_stmt *del_scopefile_stmt;

  int prepare_stat = sqlite3_prepare_v2(
      db, "DELETE FROM ScopeFile WHERE id = ?;", -1, &del_scopefile_stmt, NULL);

  if (prepare_stat != SQLITE_OK) {
    return (Result){.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  sqlite3_bind_int64(del_scopefile_stmt, 1, id);

  int del_scopefile_stat = sqlite3_step(del_scopefile_stmt);
  if (del_scopefile_stat != SQLITE_DONE) {
    int finalize_stat = sqlite3_finalize(del_scopefile_stmt);
    if (finalize_stat != SQLITE_OK) {
      return (Result){.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
    }
  }

  int finalize_stat = sqlite3_finalize(del_scopefile_stmt);
  if (finalize_stat != SQLITE_OK) {
    return (Result){.type = DB_ERR, .value = strdup(sqlite3_errmsg(db))};
  }

  return (Result){.type = SUCCESS, .value = ""};
}