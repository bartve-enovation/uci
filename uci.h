/*
 * libuci - Library for the Unified Configuration Interface
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LIBUCI_H
#define __LIBUCI_H

/*
 * you can use these defines to enable debugging behavior for
 * apps compiled against libuci:
 *
 * #define UCI_DEBUG_TYPECAST:
 *   enable uci_element typecast checking at run time
 *
 */

#ifdef DEBUG_ALL
#define UCI_DEBUG
#define UCI_DEBUG_TYPECAST
#endif

#include <stdbool.h>
#include <setjmp.h>
#include <stdio.h>

#define UCI_CONFDIR "/etc/config"
#define UCI_SAVEDIR "/tmp/.uci"
#define UCI_FILEMODE	0600

enum
{
	UCI_OK = 0,
	UCI_ERR_MEM,
	UCI_ERR_INVAL,
	UCI_ERR_NOTFOUND,
	UCI_ERR_IO,
	UCI_ERR_PARSE,
	UCI_ERR_DUPLICATE,
	UCI_ERR_UNKNOWN,
	UCI_ERR_LAST
};

struct uci_list;
struct uci_list
{
	struct uci_list *next;
	struct uci_list *prev;
};

struct uci_element;
struct uci_package;
struct uci_section;
struct uci_option;
struct uci_history;
struct uci_context;
struct uci_backend;
struct uci_parse_context;


/**
 * uci_parse_tuple: Parse an uci tuple
 * @ctx: uci context
 * @str: input string
 * @package: output package pointer
 * @section: output section pointer
 * @option: output option pointer
 * @value: output value pointer
 *
 * format: <package>[.<section>[.<option>]][=<value>]
 */
extern int uci_parse_tuple(struct uci_context *ctx, char *str, char **package, char **section, char **option, char **value);

/**
 * uci_alloc_context: Allocate a new uci context
 */
extern struct uci_context *uci_alloc_context(void);

/**
 * uci_free_context: Free the uci context including all of its data
 */
extern void uci_free_context(struct uci_context *ctx);

/**
 * uci_perror: Print the last uci error that occured
 * @ctx: uci context
 * @str: string to print before the error message
 */
extern void uci_perror(struct uci_context *ctx, const char *str);

/**
 * uci_import: Import uci config data from a stream
 * @ctx: uci context
 * @stream: file stream to import from
 * @name: (optional) assume the config has the given name
 * @package: (optional) store the last parsed config package in this variable
 * @single: ignore the 'package' keyword and parse everything into a single package
 *
 * the name parameter is for config files that don't explicitly use the 'package <...>' keyword
 * if 'package' points to a non-null struct pointer, enable history tracking and merge 
 */
extern int uci_import(struct uci_context *ctx, FILE *stream, const char *name, struct uci_package **package, bool single);

/**
 * uci_export: Export one or all uci config packages
 * @ctx: uci context
 * @stream: output stream
 * @package: (optional) uci config package to export
 * @header: include the package header
 */
extern int uci_export(struct uci_context *ctx, FILE *stream, struct uci_package *package, bool header);

/**
 * uci_load: Parse an uci config file and store it in the uci context
 *
 * @ctx: uci context
 * @name: name of the config file (relative to the config directory)
 * @package: store the loaded config package in this variable
 */
extern int uci_load(struct uci_context *ctx, const char *name, struct uci_package **package);

/**
 * uci_unload: Unload a config file from the uci context
 *
 * @ctx: uci context
 * @package: pointer to the uci_package struct
 */
extern int uci_unload(struct uci_context *ctx, struct uci_package *p);

/**
 * uci_lookup: Look up an uci element
 *
 * @ctx: uci context
 * @res: where to store the result
 * @package: uci_package struct 
 * @section: config section (optional)
 * @option: option to search for (optional)
 *
 * If section is omitted, then a pointer to the config package is returned
 * If option is omitted, then a pointer to the config section is returned
 */
extern int uci_lookup(struct uci_context *ctx, struct uci_element **res, struct uci_package *package, char *section, char *option);

/**
 * uci_add_section: Add an unnamed section
 * @ctx: uci context
 * @p: package to add the section to
 * @type: section type
 * @res: pointer to store a reference to the new section in
 */
extern int uci_add_section(struct uci_context *ctx, struct uci_package *p, char *type, struct uci_section **res);

/**
 * uci_set_element_value: Replace an element's value with a new one
 * @ctx: uci context
 * @element: pointer to an uci_element struct pointer
 * @value: new value
 * 
 * Only valid for uci_option and uci_section. Will replace the type string
 * when used with an uci_section
 */
extern int uci_set_element_value(struct uci_context *ctx, struct uci_element **element, char *value);

/**
 * uci_set: Set an element's value; create the element if necessary
 * @ctx: uci context
 * @package: package name
 * @section: section name
 * @option: option name
 * @value: value (option) or type (section)
 * @result: store the updated element in this variable (optional)
 */
extern int uci_set(struct uci_context *ctx, struct uci_package *p, char *section, char *option, char *value, struct uci_element **result);

/**
 * uci_rename: Rename an element
 * @ctx: uci context
 * @package: package name
 * @section: section name
 * @option: option name
 * @name: new name
 */
extern int uci_rename(struct uci_context *ctx, struct uci_package *p, char *section, char *option, char *name);

/**
 * uci_delete_element: Delete a section or option
 * @ctx: uci context
 * @e: element (section or option)
 */
extern int uci_delete_element(struct uci_context *ctx, struct uci_element *e);

/**
 * uci_delete: Delete a section or option
 * @ctx: uci context
 * @p: uci package
 * @section: section name
 * @option: option name (optional)
 */
extern int uci_delete(struct uci_context *ctx, struct uci_package *p, char *section, char *option);

/**
 * uci_save: save change history for a package
 * @ctx: uci context
 * @p: uci_package struct
 */
extern int uci_save(struct uci_context *ctx, struct uci_package *p);

/**
 * uci_commit: commit changes to a package
 * @ctx: uci context
 * @p: uci_package struct pointer
 * @overwrite: overwrite existing config data and flush history
 *
 * committing may reload the whole uci_package data,
 * the supplied pointer is updated accordingly
 */
extern int uci_commit(struct uci_context *ctx, struct uci_package **p, bool overwrite);

/**
 * uci_list_configs: List available uci config files
 * @ctx: uci context
 */
extern int uci_list_configs(struct uci_context *ctx, char ***list);

/** 
 * uci_set_savedir: override the default history save directory
 * @ctx: uci context
 * @dir: directory name
 */
extern int uci_set_savedir(struct uci_context *ctx, const char *dir);

/** 
 * uci_set_savedir: override the default config storage directory
 * @ctx: uci context
 * @dir: directory name
 */
extern int uci_set_confdir(struct uci_context *ctx, const char *dir);

/**
 * uci_add_history_path: add a directory to the search path for change history files
 * @ctx: uci context
 * @dir: directory name
 *
 * This function allows you to add directories, which contain 'overlays'
 * for the active config, that will never be committed.
 */
extern int uci_add_history_path(struct uci_context *ctx, const char *dir);

/**
 * uci_revert: revert all changes to a config item
 * @ctx: uci context
 * @p: pointer to a uci_package struct ptr (will be replaced by the revert)
 * @section: section name (optional)
 * @option option name (optional)
 */
extern int uci_revert(struct uci_context *ctx, struct uci_package **p, char *section, char *option);

/**
 * uci_parse_argument: parse a shell-style argument, with an arbitrary quoting style
 * @ctx: uci context
 * @stream: input stream
 * @str: pointer to the current line (use NULL for parsing the next line)
 * @result: pointer for the result
 */
extern int uci_parse_argument(struct uci_context *ctx, FILE *stream, char **str, char **result);

/* UCI data structures */
enum uci_type {
	UCI_TYPE_HISTORY = 0,
	UCI_TYPE_PACKAGE = 1,
	UCI_TYPE_SECTION = 2,
	UCI_TYPE_OPTION = 3,
	UCI_TYPE_PATH = 4
};

enum uci_flags {
	UCI_FLAG_STRICT =        (1 << 0), /* strict mode for the parser */
	UCI_FLAG_PERROR =        (1 << 1), /* print parser error messages */
	UCI_FLAG_EXPORT_NAME =   (1 << 2), /* when exporting, name unnamed sections */
	UCI_FLAG_SAVED_HISTORY = (1 << 3), /* store the saved history in memory as well */
};

struct uci_element
{
	struct uci_list list;
	enum uci_type type;
	char *name;
};

struct uci_backend
{
	const char *name;
	char **(*list_configs)(struct uci_context *ctx);
	struct uci_package *(*load)(struct uci_context *ctx, const char *name);
	void (*commit)(struct uci_context *ctx, struct uci_package **p, bool overwrite);
};


struct uci_context
{
	/* list of config packages */
	struct uci_list root;

	/* parser context, use for error handling only */
	struct uci_parse_context *pctx;

	/* backend for import and export */
	struct uci_backend *backend;

	/* uci runtime flags */
	enum uci_flags flags;

	char *confdir;
	char *savedir;

	/* search path for history files */
	struct uci_list history_path;

	/* private: */
	int errno;
	const char *func;
	jmp_buf trap;
	bool internal;
	char *buf;
	int bufsz;
};

struct uci_package
{
	struct uci_element e;
	struct uci_list sections;
	struct uci_context *ctx;
	bool confdir;
	char *path;

	/* private: */
	struct uci_backend *backend;
	int n_section;
	struct uci_list history;
	struct uci_list saved_history;
};

struct uci_section
{
	struct uci_element e;
	struct uci_list options;
	struct uci_package *package;
	bool anonymous;
	char *type;
};

struct uci_option
{
	struct uci_element e;
	struct uci_section *section;
	char *value;
};

enum uci_command {
	UCI_CMD_ADD,
	UCI_CMD_REMOVE,
	UCI_CMD_CHANGE,
	UCI_CMD_RENAME
};

struct uci_history
{
	struct uci_element e;
	enum uci_command cmd;
	char *section;
	char *value;
};

/* linked list handling */
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 */
#define container_of(ptr, type, member) \
	((type *) ((char *)ptr - offsetof(type,member)))


/**
 * uci_list_entry: casts an uci_list pointer to the containing struct.
 * @_type: config, section or option
 * @_ptr: pointer to the uci_list struct
 */
#define element_to(type, ptr) \
	container_of(ptr, struct uci_ ## type, e)

#define list_to_element(ptr) \
	container_of(ptr, struct uci_element, list)

/**
 * uci_foreach_entry: loop through a list of uci elements
 * @_list: pointer to the uci_list struct
 * @_ptr: iteration variable, struct uci_element
 *
 * use like a for loop, e.g:
 *   uci_foreach(&list, p) {
 *   	...
 *   }
 */
#define uci_foreach_element(_list, _ptr)		\
	for(_ptr = list_to_element((_list)->next);	\
		&_ptr->list != (_list);			\
		_ptr = list_to_element(_ptr->list.next))

/**
 * uci_foreach_entry_safe: like uci_foreach_safe, but safe for deletion
 * @_list: pointer to the uci_list struct
 * @_tmp: temporary variable, struct uci_element *
 * @_ptr: iteration variable, struct uci_element *
 *
 * use like a for loop, e.g:
 *   uci_foreach(&list, p) {
 *   	...
 *   }
 */
#define uci_foreach_element_safe(_list, _tmp, _ptr)		\
	for(_ptr = list_to_element((_list)->next),		\
		_tmp = list_to_element(_ptr->list.next);	\
		&_ptr->list != (_list);			\
		_ptr = _tmp, _tmp = list_to_element(_ptr->list.next))

/**
 * uci_list_empty: returns true if a list is empty
 * @list: list head
 */
#define uci_list_empty(list) ((list)->next == (list))

/* wrappers for dynamic type handling */
#define uci_type_history UCI_TYPE_HISTORY
#define uci_type_package UCI_TYPE_PACKAGE
#define uci_type_section UCI_TYPE_SECTION
#define uci_type_option UCI_TYPE_OPTION

/* element typecasting */
#ifdef UCI_DEBUG_TYPECAST
static const char *uci_typestr[] = {
	[uci_type_history] = "history",
	[uci_type_package] = "package",
	[uci_type_section] = "section",
	[uci_type_option] = "option",
};

static void uci_typecast_error(int from, int to)
{
	fprintf(stderr, "Invalid typecast from '%s' to '%s'\n", uci_typestr[from], uci_typestr[to]);
}

#define BUILD_CAST(_type) \
	static inline struct uci_ ## _type *uci_to_ ## _type (struct uci_element *e) \
	{ \
		if (e->type != uci_type_ ## _type) { \
			uci_typecast_error(e->type, uci_type_ ## _type); \
		} \
		return (struct uci_ ## _type *) e; \
	}

BUILD_CAST(history)
BUILD_CAST(package)
BUILD_CAST(section)
BUILD_CAST(option)

#else
#define uci_to_history(ptr) container_of(ptr, struct uci_history, e)
#define uci_to_package(ptr) container_of(ptr, struct uci_package, e)
#define uci_to_section(ptr) container_of(ptr, struct uci_section, e)
#define uci_to_option(ptr)  container_of(ptr, struct uci_option, e)
#endif

/**
 * uci_alloc_element: allocate a generic uci_element, reserve a buffer and typecast
 * @ctx: uci context
 * @type: {package,section,option}
 * @name: string containing the name of the element
 * @datasize: additional buffer size to reserve at the end of the struct
 */
#define uci_alloc_element(ctx, type, name, datasize) \
	uci_to_ ## type (uci_alloc_generic(ctx, uci_type_ ## type, name, sizeof(struct uci_ ## type) + datasize))

#define uci_dataptr(ptr) \
	(((char *) ptr) + sizeof(*ptr))

#endif
