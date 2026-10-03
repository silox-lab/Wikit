/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/

#ifndef UTILS
#define UTILS
#include <ftw.h>

char *get_user();
int remove_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
int remove_directory_recursive(const char *path);

#endif