/*
 * Webcom C SDK
 *
 * Copyright 2017 Orange
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

#define _GNU_SOURCE
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <webcom-c/webcom.h>
#include <webcom-c/webcom-libev.h>
#include <ev.h>
#include <ncurses.h>
#include <json-c/json.h>

static void on_connected(wc_context_t *ctx, int initial_connection);
static void on_disconnected(wc_context_t *ctx);
void stdin_cb (EV_P_ ev_io *w, int revents);
void print_new_message(json_object *message);
void process_message_data_update(wc_context_t *cnx, ws_on_data_event_t event, char *path, char *json_data, void *param);

WINDOW *chat, *chatbox;
WINDOW *input, *inputbox;
char *name = NULL;
const char *escaped_name;

int main(int argc, char *argv[]) {
	wc_context_t *ctx;
	int wc_fd;
	struct wc_libev_integration eli;
	struct ev_loop *loop = EV_DEFAULT;
	ev_io stdin_watcher;
	int opt;
	json_object *json_name;

	while ((opt = getopt(argc, argv, "n:h")) != -1) {
		switch((char)opt) {
		case 'n':
			name = optarg;
			break;
		case 'h':
			printf(
					"%s [OPTIONS]\n"
					"Note: quit the chatroom by entering \"/quit\"\n"
					"Options:\n"
					"-n NICKNAME: Set your nickname in the chatroom (default: \"C-SDK-demo\")\n"
					"-h         : Displays this help message.\n",
					*argv);
			exit(0);
			break;
		}
	}

	if (name == NULL) {
		name = calloc(17,1);
		getlogin_r(name, 16);
	}

	eli.loop = loop; /* the event loop */
	eli.on_connected = on_connected; /* connection established callback */
	eli.on_disconnected = on_disconnected; /* disconnected callback */

	ctx = wc_context_new_with_libev(
			"io.datasync.orange.com",
			443,
			"chat",
			&eli);

	if (ctx == NULL) {
		return 1;
	}

	/* if stdin has data to read, call stdin_watcher() */
	ev_io_init(&stdin_watcher, stdin_cb, fileno(stdin), EV_READ);
	stdin_watcher.data = ctx;
	ev_io_start (loop, &stdin_watcher);

	initscr();

	keypad(stdscr, FALSE);

	chatbox = subwin(stdscr, LINES - 3, COLS, 0, 0);
	chat = derwin(chatbox, LINES - 5, COLS - 2, 1, 1);
	inputbox = subwin(stdscr, 3, COLS, LINES - 3, 0);
	input = derwin(inputbox, 1, COLS - 2, 1, 1);
	box(chatbox, ACS_VLINE, ACS_HLINE);
	box(inputbox, ACS_VLINE, ACS_HLINE);
	scrollok(chat, TRUE);
	wrefresh(stdscr);

	json_name = json_object_new_string(name);
	escaped_name = json_object_to_json_string(json_name);

	ev_run(loop, 0);

	endwin();

	json_object_put(json_name);
	return 0;
}

/*
 * this callback is called by the Webcom SDK once the connection was
 * established
 */
static void on_connected(wc_context_t *ctx, int initial_connection) {

	wc_on_data(ctx, "/", process_message_data_update, NULL);
	wc_req_listen(ctx, NULL, "/");

}

/*
 * called by the SDK on disconnection
 */
static void on_disconnected(wc_context_t *ctx) {
	puts("Disconnected");
	exit(1);
}

void process_message_data_update(
		UNUSED_PARAM(wc_context_t *cnx),
		UNUSED_PARAM(ws_on_data_event_t event),
		char *path,
		char *json_data,
		UNUSED_PARAM(void *param))
{
	json_object *json = json_tokener_parse(json_data);
	if (json != NULL) {
		if(strcmp(path, "/") == 0) {
			json_object_object_foreach(json, key, val) {
				UNUSED_VAR(key);
				print_new_message(val);
			}
		} else {
			print_new_message(json);
		}
	}
	json_object_put(json);
}

void print_new_message(json_object *message) {
	json_object *name;
	json_object *text;

	json_object_object_get_ex(message, "name", &name);
	json_object_object_get_ex(message, "text", &text);

	wprintw(chat, "%*s: %s\n",
			15,
			name == NULL ? "<Anonymous>" : json_object_get_string(name),
			text == NULL ? "<NULL>" : json_object_get_string(text));
	wrefresh(chat);
	werase(input);
	wrefresh(input);
}

/*
 * libev callback when data is available on stdin
 */
void stdin_cb (EV_P_ ev_io *w, UNUSED_PARAM(int revents)) {
	wc_context_t *cnx = (wc_context_t *)w->data;
	static char buf[2048];
	json_object *text;
	char *json_str;
	const char *escaped_text;

	if (wgetnstr(input, buf, sizeof(buf)) == OK) {
		if(strcmp(buf, "/quit") == 0) {
			ev_io_stop(EV_A_ w);
			ev_break(EV_A_ EVBREAK_ALL);
			wc_context_close_cnx(cnx);
		} else {
			text = json_object_new_string(buf);
			escaped_text = json_object_to_json_string(text);
			asprintf(&json_str, "{\"name\":%s,\"text\":%s}", escaped_name, escaped_text);
			wc_req_push(cnx, NULL, "/", json_str);
			free(json_str);
			json_object_put(text);
		}
	}
	if (feof(stdin)) {
		/* quit if EOF on stdin */
		ev_io_stop(EV_A_ w);
		ev_break(EV_A_ EVBREAK_ALL);
		wc_context_close_cnx(cnx);
	}
}
