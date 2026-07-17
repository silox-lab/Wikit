#ifndef ERRORS
#define ERRORS

typedef enum ResultType {
  SIMPLE_ERR,
  SUCCESS,
  FILE_DIR_NOTFOUND,
} ResultType;

typedef struct Result {
  ResultType type;
  void *value;
} Result;

#endif
