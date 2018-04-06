/*
 * webcom-sdk-c
 *
 * Copyright 2018 Orange
 * <camille.oudot@orange.com>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef LIB_DATASYNC_ON_ON_SUBSCRIPTION_H_
#define LIB_DATASYNC_ON_ON_SUBSCRIPTION_H_

#include "../path.h"
#include "../cache/treenode.h"

enum on_sub_type {
	ON_VALUE,
	ON_CHILD_ADDED,
	ON_CHILD_CHANGED,
	ON_CHILD_REMOVED,

	ON_TYPE_NUM // keep last
};

enum on_status {
	ON_STATUS_PENDING,
	ON_STATUS_WATCHING
};

typedef int(*on_value_f)(char * data);
typedef int(*on_child_added_f)(char * data, char *prev_child);
typedef int(*on_child_changed_f)(char * data, char *prev_child);
typedef int(*on_child_removed_f)(char * data);
typedef int(*on_child_f)(char * data, char *prev_child);

union on_callback {
	on_value_f on_value_cb;
	on_child_added_f on_child_added_cb;
	on_child_changed_f on_child_changed_cb;
	on_child_removed_f on_child_removed_cb;
};

struct on_sub {
	enum on_sub_type type;
	enum on_status status;
	wc_ds_path_t *path;
	struct on_sub *next;
	union on_callback cb;
};

struct on_value_sub {
	wc_ds_path_t *path;
	struct on_value_sub *next;
	on_value_f cb;
	treenode_hash_t hash;
};

struct on_child_sub {
	wc_ds_path_t *path;
	struct on_value_sub *next;
	on_child_f cb;
	treenode_hash_t hash;
};

struct on_sub *on_sub_new(enum on_sub_type type, char *path, union on_callback cb);
void on_sub_destroy(struct on_sub *sub);
void on_sub_destroy_list(struct on_sub *sub);

#endif /* LIB_DATASYNC_ON_ON_SUBSCRIPTION_H_ */
