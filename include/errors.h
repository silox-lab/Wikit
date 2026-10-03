/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#ifndef ERRORS
#define ERRORS

typedef enum ResultType {
  SIMPLE_ERR,
  SUCCESS,
  FILE_DIR_NOTFOUND,
  DB_ERR,
} ResultType;

typedef struct Result {
  ResultType type;
  void *value;
} Result;

#endif
