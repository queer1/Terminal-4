/*
 * filesys.h
 *
 *  Created on: Nov 22, 2012
 *      Author: Greg
 */

#ifndef FILESYS_H_
#define FILESYS_H_

#include <json/json.h>

json_object *filesys_get_filelist(json_object *jobj);
json_object *filesys_get_profiles();
json_object *filesys_save_profiles(json_object *jobj);

#endif /* FILESYS_H_ */
