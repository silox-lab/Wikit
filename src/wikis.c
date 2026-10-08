/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/

#include <dirent.h>
#include <stdio.h>
#include "../include/errors.h"
#include "../include/globals.h"
#include <string.h>
#include <sys/stat.h>
#include "../include/utils.h"

Result get_wiki(char *name)
{
    char wiki_fullpath[256];

    snprintf(
        wiki_fullpath,
        sizeof(wiki_fullpath),
        "%s/%s",
        GET_STORAGE_PATH(),
        name
    );

    DIR *wiki_dir = opendir(wiki_fullpath);

    if (wiki_dir == NULL) {

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

Result make_wiki(char *name) {

  char wiki_fullpath[256];
  snprintf(wiki_fullpath, sizeof(wiki_fullpath), "%s/%s", GET_STORAGE_PATH(), name);
  
  Result wiki_exist = get_wiki(name);
  
  if (wiki_exist.type == FILE_DIR_NOTFOUND) {
    
    int mkdir_e = mkdir(wiki_fullpath, S_IRWXU);
    if (mkdir_e == -1) {
        return (Result) {
        .type = SIMPLE_ERR,
        .value = "Error when creating Wiki."
        };
    }
    return (Result){.type = SUCCESS, .value = "" };
  } else {
    return (Result){.type = SIMPLE_ERR, .value = "Wiki already exist." };
  }
}

Result delete_wiki(char *name) {
  char wiki_fullpath[256];
  snprintf(wiki_fullpath, sizeof(wiki_fullpath), "%s/%s", GET_STORAGE_PATH(), name);

  Result wiki_exist = get_wiki(name);
  if (wiki_exist.type != SUCCESS) {
    return (Result){ .type = SIMPLE_ERR, .value = "Wiki not found." };
  }

  int del_dir = remove_directory_recursive(wiki_fullpath);
  if (del_dir != 0) {
    return (Result) { .type = SIMPLE_ERR, .value = "delete failed." };
  }

  return (Result) { .type = SUCCESS, .value = "" };
}

Result update_wiki(char *name, char *new_name) {
  char wiki_path[256];
  snprintf(wiki_path, sizeof(wiki_path), "%s/%s", GET_STORAGE_PATH(), name);

  Result wiki_exist = get_wiki(name);
  if (wiki_exist.type != SUCCESS) {
    return (Result){ .type = SIMPLE_ERR, .value = "Wiki not found." };
  }

  Result validate_rename = get_wiki(new_name);
  if (wiki_exist.type == SUCCESS) {
    return (Result){ .type = SIMPLE_ERR, .value = "there are already wikis with the name you want to chose." };
  }

  char new_wiki_path[256];
  snprintf(new_wiki_path, sizeof(new_wiki_path), "%s/%s", GET_STORAGE_PATH(), new_name);

  int rename_w = rename(wiki_path, new_wiki_path);
  if (rename_w != 0) {
    return (Result) { .type = SIMPLE_ERR, .value = "error in renaming wiki." };
  }

  return (Result) { .type = SUCCESS, .value = "" };
}