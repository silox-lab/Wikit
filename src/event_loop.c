/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
 */

#include "../include/schema.h"
#include "../include/database/wiki_db.h"
#include "../include/database/scope_db.h"
#include "../include/database/scopefile_db.h"
#include "../include/database/general.h"
#include "../include/globals.h"

#include <ncurses.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

void run_event_loop(void)
{

    INIT_DATABASE();

    sqlite3 *db = GET_W_DATABASE();

    if (db == NULL) {
        printf("ERROR: database is NULL\n");
        return;
    }

    printf("DB filename: %s\n",
           sqlite3_db_filename(db, "main"));

    Result r;

    r = create_table_db(DB_WIKI);

    if (r.type != SUCCESS) {
        printf("Wiki table creation failed: %s\n",
               r.value ? (char *)r.value : "unknown error");
        return;
    }

    r = create_table_db(DB_SCOPE);

    if (r.type != SUCCESS) {
        printf("Scope table creation failed: %s\n",
               r.value ? (char *)r.value : "unknown error");
        return;
    }

    r = create_table_db(DB_SCOPE_FILE);

    if (r.type != SUCCESS) {
        printf("ScopeFile table creation failed: %s\n",
               r.value ? (char *)r.value : "unknown error");
        return;
    }

    Wiki w = {
        .description = "generator guide",
        .name        = "energy"
    };

    Result make_w = create_wiki_db(&w);

    if (make_w.type != SUCCESS) {

        printf("create_wiki_db failed: %s\n",
               make_w.value
                   ? (char *)make_w.value
                   : "unknown error");

        return;
    }

    printf("Wiki created successfully.\n");


    /* ---------------------------------------------------------
     * Get the newly created Wiki
     * --------------------------------------------------------- */

    Result get_w = get_wiki_db(w.name, 0);

    if (get_w.type != SUCCESS) {

        printf("get_wiki_db failed: %s\n",
               get_w.value
                   ? (char *)get_w.value
                   : "unknown error");

        return;
    }

    if (get_w.value == NULL) {

        printf("ERROR: Wiki was created but could not be found.\n");

        return;
    }

    Wiki **wiki_arr = (Wiki **)get_w.value;

    if (wiki_arr[0] == NULL) {

        printf("ERROR: Wiki result contains no Wiki.\n");

        return;
    }

    Wiki *wiki = wiki_arr[0];

    Scope s = {
        .name            = "lithium ion",
        .parent_scope_id = 0,
        .wiki_id         = wiki->id
    };

    Result make_s = create_scope_db(&s, wiki);

    if (make_s.type != SUCCESS) {

        printf("create_scope_db failed: %s\n",
               make_s.value
                   ? (char *)make_s.value
                   : "unknown error");

        return;
    }

    printf("Top-level scope created successfully.\n");

    Result get_s =
        get_scope_db(
            0,
            wiki->id,
            0,
            s.name
        );

    if (get_s.type != SUCCESS) {

        printf("get_scope_db failed: %s\n",
               get_s.value
                   ? (char *)get_s.value
                   : "unknown error");

        return;
    }

    if (get_s.value == NULL) {

        printf("ERROR: Scope was created but could not be found.\n");

        return;
    }

    Scope **scope_arr = (Scope **)get_s.value;

    if (scope_arr[0] == NULL) {

        printf("ERROR: Scope result contains no Scope.\n");

        return;
    }

    Scope *scope1 = scope_arr[0];

    Scope s2 = {
        .name            = "ion3",
        .parent_scope_id = scope1->id,
        .wiki_id         = wiki->id
    };

    Result make_s2 = create_scope_db(&s2, wiki);

    if (make_s2.type != SUCCESS) {

        printf("create_scope_db (child) failed: %s\n",
               make_s2.value
                   ? (char *)make_s2.value
                   : "unknown error");

        return;
    }

    printf("Child scope created successfully.\n");

    Result get_s2 =
        get_scope_db(
            0,
            wiki->id,
            scope1->id,
            s2.name
        );

    if (get_s2.type != SUCCESS) {

        printf("get_scope_db (child) failed: %s\n",
               get_s2.value
                   ? (char *)get_s2.value
                   : "unknown error");

        return;
    }

    if (get_s2.value == NULL) {

        printf("ERROR: Child scope was created but not found.\n");

        return;
    }

    Scope **scope2_arr = (Scope **)get_s2.value;

    if (scope2_arr[0] == NULL) {

        printf("ERROR: Child scope result contains no Scope.\n");

        return;
    }

    Scope *scope2 = scope2_arr[0];
    
    ScopeFile sf = {
        .name       = "hello_world",
        .scope_id   = scope2->id,
        .extension  = "txt"
    };

    Result make_sf = create_scopefile_db(&sf);

    if (make_sf.type != SUCCESS) {

        printf("create_scopefile_db failed: %s\n",
               make_sf.value
                   ? (char *)make_sf.value
                   : "unknown error");

        return;
    }

    printf("ScopeFile created successfully.\n");

    Result get_sf =
        get_scopefile_db(
            sf.name,
            sf.scope_id
        );

    if (get_sf.type != SUCCESS) {

        printf("get_scopefile_db failed: %s\n",
               get_sf.value
                   ? (char *)get_sf.value
                   : "unknown error");

        return;
    }

    if (get_sf.value == NULL) {

        printf(
            "ERROR: ScopeFile was created but could not be found.\n"
        );

        return;
    }

    ScopeFile *sf_from_db =
        ((ScopeFile **)get_sf.value)[0];

    sqlite3_close(GET_W_DATABASE());
}