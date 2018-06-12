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


#include <json-c/json.h>

#include "webcom-c/webcom.h"
#include "../path.h"
#include "../cache/treenode_cache.h"
#include "../listen/listen_registry.h"
#include "on_registry.h"


void wc_datasync_on_value(wc_context_t *ctx, char *path, on_callback_f callback) {
	on_registry_add(ctx, ON_VALUE, path, callback);
	wc_datasync_watch(ctx, path);
}


void wc_datasync_on_child_added(wc_context_t *ctx, char *path, on_callback_f callback) {
	on_registry_add(ctx, ON_CHILD_ADDED, path, callback);
	wc_datasync_watch(ctx, path);
}

void wc_datasync_on_child_changed(wc_context_t *ctx, char *path, on_callback_f callback) {
	on_registry_add(ctx, ON_CHILD_CHANGED, path, callback);
	wc_datasync_watch(ctx, path);
}

void wc_datasync_on_child_removed(wc_context_t *ctx, char *path, on_callback_f callback) {
	on_registry_add(ctx, ON_CHILD_REMOVED, path, callback);
	wc_datasync_watch(ctx, path);
}

static void wc_datasync_off_w(wc_context_t *ctx, char *path, int event_mask, on_callback_f cb) {
	int removed;
	wc_ds_path_t *parsed_path = wc_datasync_path_new(path);
	removed = on_registry_remove(ctx, parsed_path, event_mask, cb);
	wc_datasync_unwatch_ex(ctx, parsed_path, removed);
	wc_datasync_path_destroy(parsed_path);
	(void)removed;
}

void wc_datasync_off_path(wc_context_t *ctx, char *path) {
	wc_datasync_off_w(ctx, path, -1, NULL);
}

void wc_datasync_off_path_type(wc_context_t *ctx, char *path, enum on_event_type type) {
	wc_datasync_off_w(ctx, path, 1 << type, NULL);
}

void wc_datasync_off_path_type_cb(wc_context_t *ctx, char *path, enum on_event_type type, on_callback_f cb) {
	wc_datasync_off_w(ctx, path, 1 << type, cb);
}
