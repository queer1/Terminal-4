/*
 * xml.c
 *
 *  Created on: Nov 21, 2012
 *      Author: gregb_000
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define QUOTE_CHAR ((char)34)

int xml_is_element(const char *xml, const char *name) {
	int rc = 0;
	int len = strlen(name);

	while (1) {
		char c = *xml++;
		if (c == 0) {
			break;
		} else if (c == '<') {
			if (memcmp(xml, name, len) == 0
					&& (xml[len] == ' ' || xml[len] == '>')) {
				rc = 1;
				break;
			}
		}
	}

	return rc;
}

int xml_get_attr(const char *xml, const char *name, char *buf) {
	int rc = -1;
	int len = strlen(name);
	buf[0] = 0;
	while (1) {
		char c = *xml++;
		if (c == 0) {
			break;
		} else if (c == ' ' && memcmp(xml, name, len) == 0 && xml[len] == '=') {
			xml += len;
			xml++; //skip = char
			assert(*xml == QUOTE_CHAR);
			//make sure the next char is the quote char
			xml++; //skip quote char
			while (*xml && *xml != QUOTE_CHAR ) {
				*buf++ = *xml++;
			}

			*buf = 0;
			rc = 0;
			break;
		}
	}

	return rc;
}

int xml_get_attr_asint(const char *xml, const char *name) {
	int rc = 0;
	char buf[1024];

	if (xml_get_attr(xml, name, buf) == 0) {
		rc = atoi(buf);
	}

	return rc;
}

int xml_get_attr_asbool(const char *xml, const char *name) {
	int rc = 0;
	char buf[1024];

	if (xml_get_attr(xml, name, buf) == 0) {
		if (strcmp(buf, "true") == 0) {
			rc = 1;
		}
	}

	return rc;
}

int xml_get_attr_ismatch(const char *xml, const char *name, const char *match) {
	int rc = 0;
	char buf[256];

	if (xml_get_attr(xml, name, buf) == 0) {
		if (strcmp(buf, match) == 0) {
			return 1;
		}
	}

	return rc;
}

int xml_get_innertext(const char *xml, char *buf) {
	int rc = -1;
	buf[0] = 0;

	while (1) {
		char c = *xml++;
		if (c == 0) {
			break;
		} else if (c == '>') {
			int len = 0;
			while (*xml && *xml != '<') {
				if (memcmp(xml, "&amp;", 5) == 0) {
					buf[len++] = '&';
					xml += 5;
				} else {
					buf[len++] = *xml++;
				}
			}

			if (len > 0) {
				buf[len] = 0;
				rc = 0;
				break;
			}
		}
	}

	return rc;
}

