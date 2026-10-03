/*
 * SPDX-FileCopyrightText: 2026 silox-lab
 *
 * SPDX-License-Identifier: MIT
*/
#ifndef SLUGIFY
#define SLUGIFY
#include <stddef.h>

void slugify(const char *input, char *output, size_t output_size, char separator);
void slugify_default(const char *input, char *output, size_t output_size);

#endif