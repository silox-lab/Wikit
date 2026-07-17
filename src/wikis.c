#include <dirent.h>
#include <stdio.h>
#include "../include/errors.h"
#include "../include/globals.h"
#include <sys/stat.h>


Result get_wiki(char *name) {
  
  char wiki_fullpath[256];
  snprintf(wiki_fullpath, sizeof(wiki_fullpath), "%s%s", GET_WSTORAGE_PATH(), name);
  
  DIR *wiki_dir = opendir(wiki_fullpath);

  if (wiki_dir != NULL) {
     return (Result){.type = FILE_DIR_NOTFOUND, .value = "directory not found" };
  } else {
    closedir(wiki_dir);
    return (Result){.type = SUCCESS, .value = wiki_dir};
  }
  
}

Result make_wiki(char *name) {

  char wiki_fullpath[256];
  snprintf(wiki_fullpath, sizeof(wiki_fullpath), "%s%s", GET_WSTORAGE_PATH(), name);
  
  Result wiki_exist = get_wiki(name);
  
  if (wiki_exist.type != SUCCESS) {
    
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
