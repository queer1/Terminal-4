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
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

void format_time(char *buf, struct tm *time) {
	strftime(buf, 25, "%m/%d/%Y %H:%M:%S %p", time);
}

json_object *filesys_get_filelist(json_object *jobj) {
	json_object *action_data;

	DIR *d = opendir(
			json_object_get_string(json_object_object_get(jobj, "ActionData")));

	if (d) {
		action_data = json_object_new_object();
		json_object *directories = json_object_new_array();
		json_object *files = json_object_new_array();
		struct stat s;
		char date[25];

		while (1) {
			struct dirent *entry;
			entry = readdir(d);

			if (!entry) {
				json_object_object_add(action_data, "Directories", directories);
				json_object_object_add(action_data, "Files", files);
				break;
			}

			if (strcmp(entry->d_name, ".") == 0)
				continue;

			stat(entry->d_name, &s);

			memset(&date, 0, sizeof(date));
			format_time(date, localtime(&(s.st_mtime)));
			json_object *modified = json_object_new_string(date);
			json_object *name = json_object_new_string(entry->d_name);

			if (s.st_mode & S_IFDIR) {
				json_object *directory = json_object_new_object();

				json_object_object_add(directory, "Name", name);
				json_object_object_add(directory, "Modified", modified);

				json_object_array_add(directories, directory);
			} else {
				json_object *file = json_object_new_object();
				json_object *size = json_object_new_int(s.st_size);

				json_object_object_add(file, "Name", name);
				json_object_object_add(file, "Modified", modified);
				json_object_object_add(file, "Size", size);

				json_object_array_add(files, file);
			}
		}

		if (closedir(d)) {
		}

	} else {
		action_data = json_object_new_string("Failed to open directory");
	}

	json_object_object_add(jobj, "ActionData", action_data);
	return jobj;
}

json_object *filesys_delete_file(json_object *jobj) {
	json_object *action_data;
	char *file;

	strcpy(file,
			json_object_get_string(json_object_object_get(jobj, "ActionData")));
	action_data = json_object_new_boolean(!remove(file));
	return action_data;
}

json_object *filesys_get_file(json_object *jobj) {
	json_object *action_data;
	char *file;

	strcpy(file,
			json_object_get_string(json_object_object_get(jobj, "ActionData")));
	json_object_object_add(jobj, "ActionData", action_data);
	return jobj;
}

json_object *filesys_get_profiles() {
	json_object *jobj;
	json_object *action;
	json_object *action_data;
	FILE *fp;

	jobj = json_object_new_object();
	action = json_object_new_string("GetProfiles");
	json_object_object_add(jobj, "Action", action);

	fp = fopen("profiles", "r+");
	if (fp) {
		action_data = json_object_from_file("profiles");
	} else {
		action_data = json_object_new_string("No profile found");
	}

	fclose(fp);
	json_object_object_add(jobj, "ActionData", action_data);

	return jobj;
}

json_object *filesys_save_profiles(json_object *jobj) {
	json_object *action_data;

	action_data = json_object_object_get(jobj, "ActionData");
	json_object_to_file("profiles", action_data);

	//TODO: Activate the profile

	return filesys_get_profiles();
}

