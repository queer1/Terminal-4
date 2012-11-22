/*
 * xml.h
 *
 *  Created on: Nov 21, 2012
 *      Author: Greg
 */

int xml_is_element(const char *xml, const char *name);
int xml_get_attr(const char *xml, const char *name, char *buf);
int xml_get_attr_asint(const char *xml, const char *name);
int xml_get_attr_asbool(const char *xml, const char *name);
int xml_get_attr_ismatch(const char *xml, const char *name, const char *match);
int xml_get_innertext(const char *xml, char *buf);
