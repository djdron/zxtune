/*
 * asap-stdio.c - stdio loader for ASAP
 *
 * Copyright (C) 2025-2026  Piotr Fusik
 *
 * This file is part of ASAP (Another Slight Atari Player),
 * see http://asap.sourceforge.net
 *
 * ASAP is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * ASAP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ASAP; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdint.h>
#include <stdio.h>

#include "asap-stdio.h"

typedef struct {
	int (*load)(const ASAPFileLoader *self, const char *filename, uint8_t *buffer, int length);
} ASAPFileLoaderVtbl;

struct ASAPFileLoader {
	const ASAPFileLoaderVtbl *vtbl;
};

static int loader_load(const ASAPFileLoader *self, const char *filename, uint8_t *buffer, int length)
{
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		return -1;
	length = (int) fread(buffer, 1, length, fp);
	fclose(fp);
	return length;
}

const ASAPFileLoader *ASAPFileLoader_GetStdio(void)
{
	static const ASAPFileLoaderVtbl loader_vtbl = { loader_load };
	static const ASAPFileLoader loader = { &loader_vtbl };
	return &loader;
}
