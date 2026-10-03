/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#include <dirent.h>
#include <stdio.h>
#include "../include/errors.h"
#include "../include/globals.h"
#include "../include/database/scope_db.h"
#include "../include/scopes.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../include/slugify.h"
#include <fcntl.h>
#include <unistd.h>

Result build_scopefile_path(ScopeFile sf) {
  
  Result scope_exist = get_scope_db(sf.scope_id, 0, 0, NULL);
  if (scope_exist.type != SUCCESS) {
    return (Result) { .type = scope_exist.type, .value = scope_exist.value };
  }
  if (scope_exist.value == NULL) {
    return (Result) { .type = SIMPLE_ERR, .value = "Scope not found." };
  }

  Scope *scope = ((Scope **)scope_exist.value)[0];

  Result scope_path_result = build_scope_path(*scope);
  if (scope_path_result.type != SUCCESS) {
    return (Result) { .type = scope_path_result.type, .value = scope_path_result.value };
  }

  char *scope_path = scope_path_result.value;

  char scopefile_name_slug[256];
  slugify_default(sf.name, scopefile_name_slug, sizeof(scopefile_name_slug));

  size_t name_cap = strlen(scopefile_name_slug) + 1
                  + (sf.extension ? strlen(sf.extension) : 0) + 1;
  char *filename = calloc(name_cap, 1);
  if (!filename) {
    free(scope_path);
    return (Result){ .type = SIMPLE_ERR, .value = "oom" };
  }
  snprintf(filename, name_cap, "%s.%s",
           scopefile_name_slug, sf.extension ? sf.extension : "");

  size_t cap = strlen(scope_path) + 1 + strlen(filename) + 1;
  char *dirs_str = calloc(cap, 1);
  if (!dirs_str) {
    free(filename);
    free(scope_path);
    return (Result){ .type = SIMPLE_ERR, .value = "oom" };
  }
  snprintf(dirs_str, cap, "%s/%s", scope_path, filename);

  free(filename);
  free(scope_path);

  free(scope->name);
  free(scope);

  return (Result) { .type = SUCCESS, .value = dirs_str };
}

Result get_scopefile(char *path, char *name) {
  
  char sf_fullpath[256];
  snprintf(sf_fullpath, sizeof(sf_fullpath), "%s/%s/%s", GET_STORAGE_PATH(), path, name);
  
  int sf = open(sf_fullpath, O_RDONLY);
  if (sf == -1) {
    return (Result) { .type = SIMPLE_ERR, .value = "error in get scopefile." };
  }

  return (Result) { .type = SUCCESS, .value = &sf };
}

Result delete_scopefile(char *path, char *name) {
  char sf_fullpath[256];
  snprintf(sf_fullpath, sizeof(sf_fullpath), "%s/%s", GET_STORAGE_PATH(), name);

  Result sf_exist = get_scopefile(path, name);
  if (sf_exist.type != SUCCESS) {
    return (Result) { .type = SIMPLE_ERR, .value = "ScopeFile not found." };
  }

  if (unlink(sf_fullpath) != 0) {
    return (Result) { .type = SIMPLE_ERR, .value = "delete ScopeFile failed." };
  }

  return (Result) { .type = SUCCESS, .value = "" };
}

Result make_scopefile(ScopeFile sf) {

  char path[256];

  Result get_scope = get_scope_db(sf.scope_id, 0, 0, NULL);
  if (get_scope.type != SUCCESS) {
    return (Result) { .type = get_scope.type, .value = get_scope.value };
  }

  Result get_scope_path = build_scope_path(*((Scope **)get_scope.value)[0]);
  if (get_scope_path.type != SUCCESS) {
    return (Result) { .type = get_scope_path.type, .value = get_scope_path.value };
  }

  snprintf(path, sizeof(path), "/%s/%s.%s", (char *)get_scope_path.value, sf.name, sf.extension);

  int sf_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);  if (sf_fd == -1) {
    return (Result) { .type = SIMPLE_ERR, .value = "problem in making scopefile." };
  }

  if (close(sf_fd) == -1) {
    return (Result) { .type = SIMPLE_ERR, .value = "problem in making scopefile." };
  }

  return (Result) { .type = SUCCESS, .value = "" };
}