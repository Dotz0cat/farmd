/*
Copyright 2023 Dotz0cat

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

#ifndef LOOP_PRIVATE_H
#define LOOP_PRIVATE_H

#include "loop.h"

static void set_callbacks(struct evhttp *base, loop_context *context);

static void sig_int_quit_term_cb(evutil_socket_t sig, short events, void *user_data);
static void sighup_cb(evutil_socket_t sig, short events, void *user_data);
static void sigusr1_cb(evutil_socket_t sig, short events, void *user_data);
static void sigusr2_cb(evutil_socket_t sig, short events, void *user_data);

static struct evhttp_bound_socket *make_http_socket(loop_context *context);
static struct evhttp_bound_socket *make_https_socket(loop_context *context);

static struct bufferevent *make_ssl_bufferevent(struct event_base *base, void *user_data);

static void generic_http_cb(struct evhttp_request *req, void *arg);

static void barn_query_cb(struct evhttp_request *req, void *arg);
static void silo_query_cb(struct evhttp_request *req, void *arg);

static void create_save_cb(struct evhttp_request *req, void *arg);
static void open_save_cb(struct evhttp_request *req, void *arg);
static void close_save_cb(struct evhttp_request *req, void *arg);
static void ping_save_cb(struct evhttp_request *req, void *arg);

static int open_save(const char *file_name, loop_context *context);
static int create_save(const char *file_name, loop_context *context);
static int close_save(loop_context *context);
static int ping_save(const char *filename);

static void get_barn_allocation_cb(struct evhttp_request *req, void *arg);
static void get_silo_allocation_cb(struct evhttp_request *req, void *arg);

static void get_money_cb(struct evhttp_request *req, void *arg);
static void get_level_cb(struct evhttp_request *req, void *arg);
static void get_xp_cb(struct evhttp_request *req, void *arg);
static void get_skill_points_cb(struct evhttp_request *req, void *arg);
static void get_skill_status_cb(struct evhttp_request *req, void *arg);
static void get_version_cb(struct evhttp_request *req, void *arg);

static void get_barn_max_cb(struct evhttp_request *req, void *arg);
static void get_silo_max_cb(struct evhttp_request *req, void *arg);

static void fields_cb(struct evhttp_request *req, void *arg);
static void fields_harvest_cb(struct evhttp_request *req, void *arg);
static void plant_cb(struct evhttp_request *req, void *arg);
static void field_ready_cb(evutil_socket_t fd, short events, void *user_data);

static void xp_check(sqlite3 *db);

static void buy_field_cb(struct evhttp_request *req, void *arg);
static void buy_tree_plot_cb(struct evhttp_request *req, void *arg);
static void buy_skill_cb(struct evhttp_request *req, void *arg);

static void plant_tree_cb(struct evhttp_request *req, void *arg);

static void tree_mature_cb(evutil_socket_t fd, short events, void *user_data);
static void tree_harvest_ready_cb(evutil_socket_t fd, short events, void *user_data);
static void tree_harvest_cb(struct evhttp_request *req, void *arg);
static void tree_status_cb(struct evhttp_request *req, void *arg);

static void buy_item_cb(struct evhttp_request *req, void *arg);
static void sell_item_cb(struct evhttp_request *req, void *arg);
static void item_buy_price_cb(struct evhttp_request *req, void *arg);
static void item_sell_price_cb(struct evhttp_request *req, void *arg);

static void get_barn_level_cb(struct evhttp_request *req, void *arg);
static void get_silo_level_cb(struct evhttp_request *req, void *arg);
static void upgrade_barn_cb(struct evhttp_request *req, void *arg);
static void upgrade_silo_cb(struct evhttp_request *req, void *arg);

#endif /* LOOP_PRIVATE_H */
