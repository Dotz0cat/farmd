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

#define TEST_METHOD(a, b) \
if (evhttp_request_get_command(a) != b) { \
    evhttp_send_reply(a, HTTP_INTERNAL, "Client", NULL); \
    return; \
}

#define GET_QUERY(req, query) \
const struct evhttp_uri *uri_struct = evhttp_request_get_evhttp_uri(req); \
query = evhttp_uri_get_query(uri_struct);

#define GET_POST_ARG_IF_NO_QUERY(query, post_arg, req) \
if (query == NULL) { \
    post_arg = get_post_args(req); \
    if (post_arg == NULL) { \
        struct evbuffer *buf = evbuffer_new(); \
        evbuffer_add_printf(buf, "no query or post args\r\n"); \
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", buf); \
        evbuffer_free(buf); \
        return; \
    } \
    query = post_arg; \
}

#define SEND_REPLY(req, fmt) \
struct evbuffer *returnbuffer = evbuffer_new(); \
evbuffer_add_printf(returnbuffer, fmt); \
evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer); \
evbuffer_free(returnbuffer);

#define SEND_REPLY_ERROR(req, fmt) \
struct evbuffer *returnbuffer = evbuffer_new(); \
evbuffer_add_printf(returnbuffer, fmt); \
evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer); \
evbuffer_free(returnbuffer);

#define SEND_REPLY_WITH_ARG(req, fmt, arg) \
struct evbuffer *returnbuffer = evbuffer_new(); \
evbuffer_add_printf(returnbuffer, fmt, arg); \
evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer); \
evbuffer_free(returnbuffer);

#define SEND_REPLY_ERROR_WITH_ARG(req, fmt, arg) \
struct evbuffer *returnbuffer = evbuffer_new(); \
evbuffer_add_printf(returnbuffer, fmt, arg); \
evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer); \
evbuffer_free(returnbuffer);

static void set_callbacks(struct evhttp *base, loop_context *context);

static void sig_int_quit_term_cb(evutil_socket_t sig, short events, void *user_data);
static void sighup_cb(evutil_socket_t sig, short events, void *user_data);
static void sigusr1_cb(evutil_socket_t sig, short events, void *user_data);
static void sigusr2_cb(evutil_socket_t sig, short events, void *user_data);

static struct evhttp_bound_socket *make_socket(struct evhttp *evhttp_base, int port);
static struct evhttp_bound_socket *make_http_socket(loop_context *context);
static struct evhttp_bound_socket *make_https_socket(loop_context *context);
static void new_http(loop_context *context);
static void new_https(loop_context *context);
static void reload_socket(struct evhttp *base, struct evhttp_bound_socket **sock, int port, int old_port);

static struct bufferevent *make_ssl_bufferevent(struct event_base *base, void *user_data);

static void reload_ssl_keys(SSL_CTX *ctx, const char *new_pub_key, const char *old_pub_key, const char *new_priv_key, const char *old_priv_key);

static void generic_http_cb(struct evhttp_request *req, void *arg);

static char *get_post_args(struct evhttp_request *req);

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

static void field_status_cb(struct evhttp_request *req, void *arg);
static void field_harvest_cb(struct evhttp_request *req, void *arg);
static void plant_cb(struct evhttp_request *req, void *arg);
static void field_ready_cb(evutil_socket_t fd, short events, void *user_data);

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
