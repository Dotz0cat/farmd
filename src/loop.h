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
#include <math.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "save.h"
#include "list.h"
#include "inital_values.h"

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
};

typedef struct _loop_context loop_context;

struct _loop_context {
    int number;

    events_box* event_box;

    sqlite3* db;

    fields_list* field_list;
};

void loop_run(loop_context* context);

static void sig_int_quit_term_cb(evutil_socket_t sig, short events, void* user_data);
static void sighup_cb(evutil_socket_t sig, short events, void* user_data);
static void sigusr1_cb(evutil_socket_t sig, short events, void* user_data);
static void sigusr2_cb(evutil_socket_t sig, short events, void* user_data);

static void generic_http_cb(struct evhttp_request* req, void* arg);

static void barn_query_cb(struct evhttp_request* req, void* arg);
static void silo_query_cb(struct evhttp_request* req, void* arg);

static void create_save_cb(struct evhttp_request* req, void* arg);
static void open_save_cb(struct evhttp_request* req, void* arg);
static void close_save_cb(struct evhttp_request* req, void* arg);

static void get_barn_allocation_cb(struct evhttp_request* req, void* arg);
static void get_silo_allocation_cb(struct evhttp_request* req, void* arg);

static void get_money_cb(struct evhttp_request* req, void* arg);
static void get_level_cb(struct evhttp_request* req, void* arg);
static void get_xp_cb(struct evhttp_request* req, void* arg);

static void fields_cb(struct evhttp_request* req, void* arg);
static void fields_harvest_cb(struct evhttp_request* req, void* arg);
static void plant_cb(struct evhttp_request* req, void* arg);
static void field_ready_cb(evutil_socket_t fd, short events, void* user_data);

static void xp_check(sqlite3* db);

static void buy_field_cb(struct evhttp_request* req, void* arg);
static void buy_tree_plot_cb(struct evhttp_request* req, void* arg);

#endif /* LOOP_H */
