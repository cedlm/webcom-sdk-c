#ifndef INCLUDE_WEBCOM_C_WEBCOM_CNX_H_
#define INCLUDE_WEBCOM_C_WEBCOM_CNX_H_

#include <stddef.h>

#include "webcom-msg.h"

typedef struct wc_cnx wc_cnx_t;
typedef enum {
	WC_EVENT_ON_CNX_CLOSED,
	WC_EVENT_ON_CNX_ESTABLISHED,
	WC_EVENT_ON_MSG_RECEIVED,
} wc_event_t;

typedef void (*wc_on_event_cb_t) (wc_event_t event, wc_cnx_t *cnx, void *data, size_t len, void *user);

wc_cnx_t *wc_cnx_new(char *endpoint, int port, char *path, wc_on_event_cb_t callback, void *user);

void wc_cnx_free(wc_cnx_t *cnx);

void wc_cnx_connect(wc_cnx_t *cnx);

int wc_cnx_get_fd(wc_cnx_t *cnx);

int wc_cnx_close(wc_cnx_t *cnx);

void wc_service(wc_cnx_t *cnx);

#endif /* INCLUDE_WEBCOM_C_WEBCOM_CNX_H_ */
