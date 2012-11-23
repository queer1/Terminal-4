/*
 * fileys.c
 *
 *  Created on: Nov 22, 2012
 *      Author: Greg
 */

#include "filesys.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

char *filesys_get(char *dir)
{
	DIR *d = opendir(dir);

	if (!d) {
		return "Directory not found or can not be opened";
	}

	while (1) {
		struct dirent *entry;
		entry = readdir(d);
		if (!entry) {
			break;
		}
		printf ("%s\n", entry->d_name);
	}

    if (closedir (d)) {
    	//TODO: Log;
    }

    return NULL;
}

