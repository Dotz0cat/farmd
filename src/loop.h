/*
Copyright 2022-2023 Dotz0cat

This file is part of farmd.

    farmd is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    farmd is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with farmd.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LOOP_H
#define LOOP_H

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include <math.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <openssl/ssl.h>

#include "save.h"
#include "list.h"
#include "inital_values.h"
#include "config.h"

#include "box_struct.h"

#include "barn.h"
#include "silo.h"
#include "field.h"
#include "tree.h"
#include "econ.h"
#include "skill.h"

typedef struct _events_box events_box;

struct _events_box {
    struct event_base *base;
    struct event *signal_sigquit;
    struct event *signal_sigterm;
    struct event *signal_sigint;
    struct event *signal_sighup;
    struct event *signal_sigusr1;
    struct event *signal_sigusr2;
    struct evhttp *http_base;
    struct evhttp *https_base;
    struct evhttp_bound_socket *http_socket;
    struct evhttp_bound_socket *https_socket;
};

typedef struct _loop_context loop_context;

struct _loop_context {
    pre_init_stuff *pre_init_info;

    events_box *event_box;

    sqlite3 *db;

    fields_list *field_list;

    trees_list *tree_list;

    SSL_CTX *ssl_ctx;
};

void loop_run(loop_context *context);

#endif /* LOOP_H */
