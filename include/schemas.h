#ifndef SCHEMAS
#define SCHEMAS
#include <time.h>

typedef struct Sfile {
  char *name;
  char *extension;
  char *path;
} Sfile;

typedef struct Scope {
  char *label;
  char *text;
  Sfile **sfiles_list;
  time_t created_on;
  Scope *scope;
} Scope;

typedef struct Wiki {
  char *name;
  char *description;
  time_t created_on;
  Scope *scopes_list;
} Wiki;

#endif
