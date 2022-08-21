/*
Copyright 2022 Dotz0cat

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

#include <event2/event.h>
#include <event2/http.h>
#include <event2/rpc.h>
#include <event2/buffer.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "save.h"

#include "rpc_file.gen.h"

EVRPC_HEADER(BarnQuery, BarnQueryRequest, BarnQueryReply)
//EVRPC_GENERATE(BarnQuery, BarnQueryRequest, BarnQueryReply)
EVRPC_HEADER(SiloQuery, SiloQueryRequest, SiloQueryReply)

typedef struct _events_box events_box;

struct _events_box {
    struct event_base* base;
    struct event* signal_sigquit;
    struct event* signal_sigterm;
    struct event* signal_sigint;
    struct event* signal_sighup;
    struct event* signal_sigusr1;
    struct event* signal_sigusr2;
    struct evhttp* http_base;
    struct evrpc_base* rpc_base;
};

typedef struct _loop_context loop_context;

struct _loop_context {
    int number;

    events_box* event_box;

    sqlite3* db;
};

void loop_run(loop_context* context);

static void sig_int_quit_term_cb(evutil_socket_t sig, short events, void* user_data);
static void sighup_cb(evutil_socket_t sig, short events, void* user_data);
static void sigusr1_cb(evutil_socket_t sig, short events, void* user_data);
static void sigusr2_cb(evutil_socket_t sig, short events, void* user_data);

static void generic_http_cb(struct evhttp_request* req, void* arg);

static void barn_query_cb(EVRPC_STRUCT(BarnQuery)* rpc, void* arg);
static void silo_query_cb(EVRPC_STRUCT(SiloQuery)* rpc, void* arg);

static void logging_cb(int severity, const char *msg);

static int rpc_hook_cb(void* thing, struct evhttp_request* request, struct evbuffer* buff, void* data);

#endif /* LOOP_H */
