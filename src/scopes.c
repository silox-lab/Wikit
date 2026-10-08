/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include "../include/errors.h"
#include "../include/globals.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../include/utils.h"
#include "../include/database/scope_db.h"
#include "../include/database/wiki_db.h"
#include "../include/slugify.h"

Result build_scope_path(Scope s) {

  Result wiki_exist = get_wiki_db(NULL, s.wiki_id);
  if (wiki_exist.type != SUCCESS) {
    return (Result) { .type = wiki_exist.type, .value = wiki_exist.value };
  }
  if (wiki_exist.value == NULL) {
    return (Result) { .type = SIMPLE_ERR, .value = "Wiki not found." };
  }

  Wiki *wiki = ((Wiki **)wiki_exist.value)[0];
  char wiki_name_slug[256];
  slugify_default(wiki->name, wiki_name_slug, sizeof(wiki_name_slug));

  if (s.parent_scope_id == 0) {
    char scope_name_slug[256];
    slugify_default(s.name, scope_name_slug, sizeof(scope_name_slug));

    char final_path[1024];
    snprintf(final_path, sizeof(final_path), "%s/%s/%s",
             GET_STORAGE_PATH(), wiki_name_slug, scope_name_slug);

    free(wiki->description);
    free(wiki->name);
    return (Result){ .type = SUCCESS, .value = strdup(final_path) };
  }

  int dirs_length = 0;
  int dirs_cap = 8;
  char **dirs = malloc(sizeof(char *) * dirs_cap);
  if (!dirs) {
    free(wiki->description);
    free(wiki->name);
    return (Result){ .type = SIMPLE_ERR, .value = "oom" };
  }

  Scope iter_scope = s;
  while (1) {

    char *slug_name = malloc(256);
    if (!slug_name) {
      for (int j = 0; j < dirs_length; j++) free(dirs[j]);
      free(dirs);
      free(wiki->description);
      free(wiki->name);
      return (Result){ .type = SIMPLE_ERR, .value = "oom" };
    }
    slugify_default(iter_scope.name, slug_name, 256);

    if (dirs_length == dirs_cap) {
      dirs_cap *= 2;
      char **tmp = realloc(dirs, sizeof(char *) * dirs_cap);
      if (!tmp) {
        free(slug_name);
        for (int j = 0; j < dirs_length; j++) free(dirs[j]);
        free(dirs);
        free(wiki->description);
        free(wiki->name);
        return (Result){ .type = SIMPLE_ERR, .value = "oom" };
      }
      dirs = tmp;
    }
    dirs[dirs_length++] = slug_name;

    if (iter_scope.parent_scope_id == 0) break;

    Result get_parent = get_scope_db(iter_scope.parent_scope_id, 0, 0, 0);
    if (get_parent.type != SUCCESS) {
      for (int j = 0; j < dirs_length; j++) free(dirs[j]);
      free(dirs);
      free(wiki->description);
      free(wiki->name);
      return (Result) { .type = get_parent.type, .value = get_parent.value };
    }
    iter_scope = *((Scope **)get_parent.value)[0];
  }

  size_t cap = strlen(GET_STORAGE_PATH()) + 1 + strlen(wiki_name_slug) + 1;
  for (int i = 0; i < dirs_length; i++)
    cap += strlen(dirs[i]) + 1;

  char *dirs_str = calloc(cap, 1);
  if (!dirs_str) {
    for (int j = 0; j < dirs_length; j++) free(dirs[j]);
    free(dirs);
    free(wiki->description);
    free(wiki->name);
    return (Result){ .type = SIMPLE_ERR, .value = "oom" };
  }

  size_t pos = 0;
  pos += snprintf(dirs_str + pos, cap - pos, "%s/%s",
                  GET_STORAGE_PATH(), wiki_name_slug);

  for (int i = dirs_length - 1; i >= 0; i--) {
    pos += snprintf(dirs_str + pos, cap - pos, "/%s", dirs[i]);
  }

  for (int j = 0; j < dirs_length; j++) free(dirs[j]);
  free(dirs);

  free(wiki->description);
  free(wiki->name);

  return (Result) { .type = SUCCESS, .value = dirs_str };
}

Result get_scope(int64_t id)
{
    char scope_fullpath[256];

    Result scope = get_scope_db(id, 0, 0, NULL);
    if (scope.type == SUCCESS && scope.value == NULL)  {
      return (Result) { .type = FILE_DIR_NOTFOUND, .value = NULL };
    }
    
    if (scope.type != SUCCESS) {
      return (Result) { .type = scope.type, .value = scope.value };
    }

    Result s_path = build_scope_path(*((Scope **)scope.value)[0]);

    DIR *scope_dir = opendir((char *)s_path.value);

    if (scope_dir == NULL) {

        return (Result){
            .type = FILE_DIR_NOTFOUND,
            .value = NULL
        };
    }

    return (Result){
        .type = SUCCESS,
        .value = ""
    };
}

Result make_scope(Scope s) {
  
  Result scope_exist = get_scope(s.id);
  if (scope_exist.type == FILE_DIR_NOTFOUND) {
    Result build_path = build_scope_path(s);
    if (build_path.type != SUCCESS) {
      return (Result) { .type = build_path.type, .value = build_path.value };
    }

    int mkdir_e = mkdir(build_path.value, S_IRWXU);
    if (mkdir_e == -1) {
        return (Result) {
        .type = SIMPLE_ERR,
        .value = "Error when creating Scope."
        };
    }
    return (Result){.type = SUCCESS, .value = "" };
  } else {
    return (Result){.type = SIMPLE_ERR, .value = "Scope already exist." };
  }
}

Result delete_scope(Scope s) {

  Result scope_exist = get_scope(s.id);
  if (scope_exist.type == FILE_DIR_NOTFOUND) {
    return (Result){ .type = SIMPLE_ERR, .value = "Scope not found." };
  }

  Result build_path = build_scope_path(s);
  if (build_path.type != SUCCESS) {
    return (Result) { .type = build_path.type, .value = build_path.value };
  }

  int del_dir = remove_directory_recursive(build_path.value);
  if (del_dir != 0) {
    return (Result) { .type = SIMPLE_ERR, .value = "delete failed." };
  }

  return (Result) { .type = SUCCESS, .value = "" };
}

Result update_scope(int64_t scope_id, char *new_name) {

  Result scope_exist_db = get_scope_db(scope_id, 0, 0, NULL);
  if (scope_exist_db.type != SUCCESS) {
    return (Result) { .type = scope_exist_db.type, .value = scope_exist_db.value };
  }

  Scope **s_arr = scope_exist_db.value;
  if (s_arr[0] == NULL && scope_exist_db.type == SUCCESS) {
    return (Result) { .type = SIMPLE_ERR, .value = "scope not found" };
  }

  Result path = build_scope_path(*s_arr[0]);
  if (path.type != SUCCESS) {
    return (Result) { .type = path.type, .value = path.value };
  }

  Result scope_exist = get_scope(scope_id);
  if (scope_exist.type != SUCCESS) {
    return (Result) { .type = scope_exist.type, .value = scope_exist.value };
  }

  Scope new_s = { 
    .id = s_arr[0]->id,
    .parent_scope_id = s_arr[0]->parent_scope_id,
    .created_on = s_arr[0]->created_on,
    .wiki_id = s_arr[0]->wiki_id,
    .name = new_name
  };

  Result new_path = build_scope_path(new_s);
  if (new_path.type != SUCCESS) {
    return (Result) { .type = new_path.type, .value = new_path.value };
  }

  Result validate_rename = get_scope(scope_id);
  if (validate_rename.type == SUCCESS) {
    return (Result) { .type = SIMPLE_ERR, .value = "there are already scopes with the name you want to chose." };
  }

  int rename_s = rename(path.value, new_path.value);
  if (rename_s != 0) {
    return (Result) { .type = SIMPLE_ERR, .value = "error in renaming scope." };
  }

  free(s_arr[0]->name);
  free(s_arr[0]);
  free(s_arr);

  free(path.value);
  free(new_path.value);

  return (Result) { .type = SUCCESS, .value = "" };
}