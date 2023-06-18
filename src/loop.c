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

#include "loop_private.h"

void loop_run(loop_context *context) {
    context->event_box = malloc(sizeof(events_box));

    context->event_box->base = event_base_new();

    if (!context->event_box->base) {
        abort();
    }

    context->db = NULL;
    context->tree_list = NULL;
    context->field_list = NULL;
    context->grain_mill_queue = NULL;

    context->event_box->http_base = NULL;
    context->event_box->https_base = NULL;

    context->ssl_ctx = NULL;

    //set up basic signals
    context->event_box->signal_sigquit = evsignal_new(context->event_box->base, SIGQUIT, sig_int_quit_term_cb, (void*) context);
    if (!context->event_box->signal_sigquit || event_add(context->event_box->signal_sigquit, NULL) < 0) abort();

    context->event_box->signal_sigterm = evsignal_new(context->event_box->base, SIGTERM, sig_int_quit_term_cb, (void*) context);
    if (!context->event_box->signal_sigterm || event_add(context->event_box->signal_sigterm, NULL) < 0) abort();

    context->event_box->signal_sigint = evsignal_new(context->event_box->base, SIGINT, sig_int_quit_term_cb, (void*) context);
    if (!context->event_box->signal_sigint || event_add(context->event_box->signal_sigint, NULL) < 0) abort();

    context->event_box->signal_sighup = evsignal_new(context->event_box->base, SIGHUP, sighup_cb, (void*) context);
    if (!context->event_box->signal_sighup || event_add(context->event_box->signal_sighup, NULL) < 0) abort();

    context->event_box->signal_sigusr1 = evsignal_new(context->event_box->base, SIGUSR1, sigusr1_cb, (void*) context);
    if (!context->event_box->signal_sigusr1 || event_add(context->event_box->signal_sigusr1, NULL) < 0) abort();

    context->event_box->signal_sigusr2 = evsignal_new(context->event_box->base, SIGUSR2, sigusr2_cb, (void*) context);
    if (!context->event_box->signal_sigusr2 || event_add(context->event_box->signal_sigusr2, NULL) < 0) abort();

    if (context->pre_init_info->settings->https_enable != 0) {
        new_https(context);

        //make http socket if http only is false
        if (context->pre_init_info->settings->https_only == 0) {
            new_http(context);
        }
    }
    else {
        new_http(context);
    }

    if (context->pre_init_info->save != NULL) {
        if (open_save(context->pre_init_info->save, context) != 0) {
            syslog(LOG_WARNING, "error opening save at %s", context->pre_init_info->save);
            abort();
        }
    }
    else if (context->pre_init_info->settings->save_location != NULL) {
        if (open_save(context->pre_init_info->settings->save_location, context) != 0) {
            syslog(LOG_WARNING, "error opening save at %s", context->pre_init_info->settings->save_location);
            abort();
        }
    }


    //run loop
    event_base_loop(context->event_box->base, EVLOOP_NO_EXIT_ON_EMPTY);

    if (context->db != NULL) {
        close_save(context);
    }

    //clean up
    event_del(context->event_box->signal_sigquit);
    event_del(context->event_box->signal_sigint);
    event_del(context->event_box->signal_sigterm);
    event_del(context->event_box->signal_sighup);
    event_del(context->event_box->signal_sigusr1);
    event_del(context->event_box->signal_sigusr2);

    event_free(context->event_box->signal_sigquit);
    event_free(context->event_box->signal_sigint);
    event_free(context->event_box->signal_sigterm);
    event_free(context->event_box->signal_sighup);
    event_free(context->event_box->signal_sigusr1);
    event_free(context->event_box->signal_sigusr2);

    if (context->event_box->http_base != NULL) {
        evhttp_free(context->event_box->http_base);
    }
    if (context->event_box->https_base != NULL) {
        evhttp_free(context->event_box->https_base);
    }

    event_base_free(context->event_box->base);

    free(context->event_box);
    if (context->ssl_ctx != NULL) {
        SSL_CTX_free(context->ssl_ctx);
    }

    return;
}

static void set_callbacks(struct evhttp *base, loop_context *context) {
    evhttp_set_gencb(base, generic_http_cb, context);

    //Thanks to caze on #c on libera for this
    struct cb_table {
        char *name;
        void (*cb)(struct evhttp_request *req, void *arg);
    };

    struct cb_table callbacks[] = {
        {"/barn/query",                 barn_query_cb},
        {"/silo/query",                 silo_query_cb},
        {"/createSave",                 create_save_cb},
        {"/openSave",                   open_save_cb},
        {"/closeSave",                  close_save_cb},
        {"/pingSave",                   ping_save_cb},
        {"/barn/allocation",            get_barn_allocation_cb},
        {"/silo/allocation",            get_silo_allocation_cb},
        {"/getMoney",                   get_money_cb},
        {"/getLevel",                   get_level_cb},
        {"/getXp",                      get_xp_cb},
        {"/getSkillPoints",             get_skill_points_cb},
        {"/getSkillStatus",             get_skill_status_cb},
        {"/version",                    get_version_cb},
        {"/barn/max",                   get_barn_max_cb},
        {"/silo/max",                   get_silo_max_cb},
        {"/field/plant",                plant_cb},
        {"/field/harvest",              field_harvest_cb},
        {"/field/status",               field_status_cb},
        {"/field/buy",                  buy_field_cb},
        {"/buy/field",                  buy_field_cb},
        {"/tree/buy",                   buy_tree_plot_cb},
        {"/buy/tree",                   buy_tree_plot_cb},
        {"/buy/skill",                  buy_skill_cb},
        {"/skill/buy",                  buy_skill_cb},
        {"/tree/plant",                 plant_tree_cb},
        {"/tree/harvest",               tree_harvest_cb},
        {"/tree/status",                tree_status_cb},
        {"/buy/item",                   buy_item_cb},
        {"/sell/item",                  sell_item_cb},
        {"/buy/price",                  item_buy_price_cb},
        {"/sell/price",                 item_sell_price_cb},
        {"/barn/level",                 get_barn_level_cb},
        {"/silo/level",                 get_silo_level_cb},
        {"/barn/upgrade",               upgrade_barn_cb},
        {"/silo/upgrade",               upgrade_silo_cb},
        {"/barn/upgrade/cost",          get_barn_upgrade_cost_cb},
        {"/silo/upgrade/cost",          get_silo_upgrade_cost_cb},
        {"/barn/upgrade/stats",         get_barn_next_level_cb},
        {"/silo/upgrade/stats",         get_silo_next_level_cb},
        {"/grainMill/buy",              buy_grain_mill_cb},
        {"/grainMill/buy/cost",         grain_mill_buy_cost_cb},
        {"/grainMill/upgrade",          grain_mill_upgrade_cb},
        {"/grainMill/upgrade/cost",     grain_mill_upgrade_cost_cb},
        {"/grainMill/upgrade/stats",    grain_mill_next_level_stats_cb},
    };

    for (int i = 0; i < (int) (sizeof(callbacks) / sizeof(callbacks[0])); i++) {
        if (evhttp_set_cb(base, callbacks[i].name, callbacks[i].cb, context)) {
            syslog(LOG_WARNING, "failed to set %s", callbacks[i].name);
            abort();
        }
    }
}

static void sig_int_quit_term_cb(evutil_socket_t sig, short events, void *user_data) {
    loop_context *context = user_data;

    struct timeval delay = {1, 0};

    event_base_loopexit(context->event_box->base, &delay);
}

static void sighup_cb(evutil_socket_t sig, short events, void *user_data) {
    loop_context *context = user_data;

    //cache port
    int http_port = context->pre_init_info->settings->http_port;
    int https_port = context->pre_init_info->settings->https_port;
    int https_only = context->pre_init_info->settings->https_only;
    int https_enable = context->pre_init_info->settings->https_enable;
    char *pub_key;
    if (context->pre_init_info->settings->pub_key != NULL) {
        pub_key  = strdup(context->pre_init_info->settings->pub_key);
    }
    else {
        pub_key = NULL;
    }
    char *priv_key;
    if (context->pre_init_info->settings->priv_key != NULL) {
        priv_key = strdup(context->pre_init_info->settings->priv_key);
    }
    else {
        priv_key = NULL;
    }

    //reload config
    free_config_settings(context->pre_init_info->settings);

    context->pre_init_info->settings = config_parse(context->pre_init_info->config, context->pre_init_info->home, context->pre_init_info->xdg_config_home);

    if (context->pre_init_info->settings->https_enable != 0) {
        //test if was already enabled
        if (https_enable == context->pre_init_info->settings->https_enable) {
            //reload socket if the port has changed
            reload_socket(context->event_box->https_base, &context->event_box->https_socket, context->pre_init_info->settings->https_port, https_port);
            reload_ssl_keys(context->ssl_ctx, context->pre_init_info->settings->pub_key, pub_key, context->pre_init_info->settings->priv_key, priv_key);
        }
        else {
            //make the socket
            new_https(context);
        }

        if (https_only == 0 && context->pre_init_info->settings->https_only == 0) {
            //reload socket if the port has changed
            reload_socket(context->event_box->http_base, &context->event_box->http_socket, context->pre_init_info->settings->http_port, http_port);
        }
        else if (https_only != 0 && context->pre_init_info->settings->https_only == 0) {
            //make a http base and registar callbacks
            new_http(context);
        }
        else if (https_only == 0 && context->pre_init_info->settings->https_only != 0) {
            //remove http base
            if (context->event_box->http_base != NULL) {
                evhttp_free(context->event_box->http_base);
                context->event_box->http_base = NULL;
            }
        }
    }
    else {
        if (https_only != 0) {
            //it was https only now its http only due to https being disabled
            new_http(context);
        }
        else {
            //reload socket if the port has changed
            reload_socket(context->event_box->http_base, &context->event_box->http_socket, context->pre_init_info->settings->http_port, http_port);
        }

        if (https_enable != 0) {
            if (context->event_box->https_base != NULL) {
                evhttp_free(context->event_box->https_base);
                context->event_box->https_base = NULL;
            }

            SSL_CTX_free(context->ssl_ctx);
        }
    }

    //save handling
    //if from the commandline don't load.
    if (context->pre_init_info->save == NULL) {
        if (context->pre_init_info->settings->save_location != NULL) {
            close_save(context);
            open_save(context->pre_init_info->settings->save_location, context);
        }
    }

    if (pub_key != NULL) {
        free(pub_key);
    }

    if (priv_key != NULL) {
        free(priv_key);
    }
}

static void sigusr1_cb(evutil_socket_t sig, short events, void *user_data) {
    return;
}

static void sigusr2_cb(evutil_socket_t sig, short events, void *user_data) {
    return;
}

static struct evhttp_bound_socket *make_socket(struct evhttp *evhttp_base, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        syslog(LOG_WARNING, "failed to make socket");
        abort();
    }

    int reuseaddr_opt_val = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt_val, sizeof(int));

    if (evutil_make_socket_nonblocking(sock) < 0) {
        syslog(LOG_WARNING, "failed to make socket nonblocking");
        abort();
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    //get port set in config
    sin.sin_port = htons(port);
    if (bind(sock, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
        syslog(LOG_WARNING, "failed to bind socket");
        abort();
    }

    if (listen(sock, 8) < 0) {
        syslog(LOG_WARNING, "listen() failed");
        abort();
    }

    return evhttp_accept_socket_with_handle(evhttp_base, sock);
}

static struct evhttp_bound_socket *make_http_socket(loop_context *context) {
    if (context->event_box->http_base == NULL) {
        context->event_box->http_base = evhttp_new(context->event_box->base);
        if (!context->event_box->http_base) {
            syslog(LOG_WARNING, "failed to make evhttp base");
            abort();
        }
    }

    return make_socket(context->event_box->http_base, context->pre_init_info->settings->http_port);
}

static struct evhttp_bound_socket *make_https_socket(loop_context *context) {
    if (context->event_box->https_base == NULL) {
        context->event_box->https_base = evhttp_new(context->event_box->base);
        if (!context->event_box->https_base) {
            syslog(LOG_WARNING, "failed to make evhttp base");
            abort();
        }
    }

    return make_socket(context->event_box->https_base, context->pre_init_info->settings->https_port);
}

static void new_http(loop_context *context) {
    context->event_box->http_socket = make_http_socket(context);
    if (context->event_box->http_socket == NULL) {
        syslog(LOG_WARNING, "evhttp_accept_socket() for http failed");
        abort();
    }

    set_callbacks(context->event_box->http_base, context);
}

static void new_https(loop_context *context) {
    context->event_box->https_socket = make_https_socket(context);
    if (context->event_box->https_socket == NULL) {
        syslog(LOG_WARNING, "evhttp_accept_socket() for https failed");
        abort();
    }

    //ssl stuff
    context->ssl_ctx = SSL_CTX_new(TLS_server_method());
    reload_ssl_keys(context->ssl_ctx, context->pre_init_info->settings->pub_key, NULL, context->pre_init_info->settings->priv_key, NULL);

    evhttp_set_bevcb(context->event_box->https_base, make_ssl_bufferevent, context);
    set_callbacks(context->event_box->https_base, context);
}

static void reload_socket(struct evhttp *base, struct evhttp_bound_socket **sock, int port, int old_port) {
    if (old_port != port) {
        struct evhttp_bound_socket *socket = make_socket(base, port);

        evhttp_del_accept_socket(base, *sock);

        *sock = socket;
    }
}

static struct bufferevent *make_ssl_bufferevent(struct event_base *base, void *user_data) {
    loop_context *context = user_data;

    struct evhttp_bound_socket *sock = context->event_box->https_socket;

    SSL *ssl = SSL_new(context->ssl_ctx);

    struct bufferevent *bev = bufferevent_openssl_socket_new(base, evhttp_bound_socket_get_fd(sock), ssl, BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_openssl_set_allow_dirty_shutdown(bev, 1);
    
    return bev;
}

static void reload_ssl_keys(SSL_CTX *ctx, const char *new_pub_key, const char *old_pub_key, const char *new_priv_key, const char *old_priv_key) {
    if (old_pub_key != NULL && new_pub_key != NULL) {
        if (strcmp(old_pub_key, new_pub_key) != 0) {
            // if they are not equal and not null set
            SSL_CTX_use_certificate_file(ctx, new_pub_key, SSL_FILETYPE_PEM);
        }
    }
    else if (new_pub_key != NULL) {
        SSL_CTX_use_certificate_file(ctx, new_pub_key, SSL_FILETYPE_PEM);
    }

    if (old_priv_key != NULL && new_priv_key != NULL) {
        if (strcmp(old_priv_key, new_priv_key) != 0) {
            // if they are not equal and not null set
            SSL_CTX_use_PrivateKey_file(ctx, new_priv_key, SSL_FILETYPE_PEM);
        }
    }
    else if (new_priv_key != NULL) {
        SSL_CTX_use_PrivateKey_file(ctx, new_priv_key, SSL_FILETYPE_PEM);
    }

    if (SSL_CTX_check_private_key(ctx)) {
        syslog(LOG_WARNING, "ssl private key does not match");
    }
}

static void generic_http_cb(struct evhttp_request *req, void *arg) {
    struct evbuffer *returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "nothing available at %s\r\n", evhttp_request_get_uri(req));
    evhttp_send_reply(req, HTTP_NOTFOUND, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static char *get_post_args(struct evhttp_request *req) {
    //check for post parameters
    char *post_arg;
    struct evbuffer *inputbuffer = evhttp_request_get_input_buffer(req);
    size_t buffersize = evbuffer_get_length(inputbuffer);
    buffersize++;
    post_arg = malloc(buffersize);
    if (post_arg == NULL) {
        return NULL;
    }
    evbuffer_copyout(inputbuffer, post_arg, buffersize);
    post_arg[buffersize - 1] = '\0';
    //check if filled
    if (strcmp(post_arg, "") == 0) {
        if (post_arg != NULL) {
            free(post_arg);
        }
        return NULL;
    }

    return post_arg;
}

static void barn_query_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    const char *query;
    GET_QUERY(req, query)
    
    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = barn_query(context->db, query, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void silo_query_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)
    
    const char *query;
    GET_QUERY(req, query)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = silo_query(context->db, query, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void create_save_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    if (context->db != NULL) {
        SEND_REPLY_ERROR(req, "save open\r\n")
        return;
    }

    const char *file_name;
    GET_QUERY(req, file_name)

    char *filename = NULL;
    GET_POST_ARG_IF_NO_QUERY(file_name, filename, req)

    int rc = create_save(file_name, context);
    switch (rc) {
        case (1): {
            SEND_REPLY_ERROR_WITH_ARG(req, "failed to create new save at %s\r\n", file_name)
            if (filename != NULL) {
                free(filename);
            }
            return;
            break;
        }
        case (2): {
            SEND_REPLY_ERROR_WITH_ARG(req, "failed to open new save at %s for inital settings\r\n", file_name)
            if (filename != NULL) {
                free(filename);
            }
            return;
            break;
        }
        case (3): {
            SEND_REPLY_ERROR(req, "failed to add inital settings\r\n")
            if (filename != NULL) {
                free(filename);
            }
            return;
            break;
        }
    }

    SEND_REPLY_WITH_ARG(req, "new save created at %s\r\n", file_name)

    if (filename != NULL) {
        free(filename);
    }
}

static void open_save_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    if (context->db != NULL) {
        SEND_REPLY_ERROR(req, "save already open\r\n")
        return;
    }

    const char *file_name;
    GET_QUERY(req, file_name)

    char *filename = NULL;
    GET_POST_ARG_IF_NO_QUERY(file_name, filename, req)

    int rc = open_save(file_name, context);
    if (rc != 0) {
        SEND_REPLY_ERROR_WITH_ARG(req, "failed to open save at %s\r\n", file_name)
        if (filename != NULL) {
            free(filename);
        }
        return;
    }

    if (filename != NULL) {
        free(filename);
    }

    SEND_REPLY(req, "save opened\r\n")
}

static void close_save_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    if (context->db == NULL) {
        SEND_REPLY_ERROR(req, "no save open\r\n")
        return;
    }

    close_save(context);

    SEND_REPLY(req, "save closed\r\n")
}

static void ping_save_cb(struct evhttp_request *req, void *arg) {
    TEST_METHOD(req, EVHTTP_REQ_POST)

    //save open check here if implented

    const char *file_name;
    GET_QUERY(req, file_name)

    char *filename = NULL;

    GET_POST_ARG_IF_NO_QUERY(file_name, filename, req)

    if (ping_save(file_name) != 0) {
        if (filename != NULL) {
            free(filename);
        }
        SEND_REPLY_ERROR(req, "error pinging save\r\n")
        return;
    }

    if (filename != NULL) {
        free(filename);
    }

    SEND_REPLY(req, "save pinged\r\n")
}

static int open_save(const char *file_name, loop_context *context) {
    int rc = open_save_db(file_name, &context->db);
    if (rc != 0) {
        return rc;
    }

    populate_fields(context->db, &context->field_list, context->event_box->base, field_ready_cb);

    populate_trees(context->db, &context->tree_list, context->event_box->base, tree_mature_cb, tree_harvest_ready_cb);

    populate_grain_mill(context->db, &context->grain_mill_queue, context->event_box->base, NULL);

    return 0;
}

static int create_save(const char *file_name, loop_context *context) {
    int rc = create_save_db(file_name);
    if (rc != 0) {
        return 1;
    }

    rc = open_save_db(file_name, &context->db);
    if (rc != 0) {
        return 2;
    }

    rc = add_inital_save_values(context->db);
    if (rc != 0) {
        return 3;
    }

    close_save_db(context->db);
    context->db = NULL;

    return 0;
}

static int close_save(loop_context *context) {
    close_save_db(context->db);
    context->db = NULL;

    //free fields
    free_fields(&context->field_list);

    //free trees
    free_trees(&context->tree_list);

    free_queue_list(&context->grain_mill_queue);

    return 0;
}

static int ping_save(const char *filename) {
    sqlite3 *db;
    int rc = open_save_db(filename, &db);

    if (rc != 0) {
        return -1;
    }

    ping_fields(db);

    ping_trees(db);

    ping_grain_mill(db);

    close_save_db(db);

    return 0;
}

static void get_barn_allocation_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = barn_allocation(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_silo_allocation_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = silo_allocation(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_money_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = view_money(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_level_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = view_level(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_xp_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = view_xp(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_skill_points_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = view_skill_points(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_skill_status_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    const char *query;
    GET_QUERY(req, query)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = skill_status(context->db, query, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_version_cb(struct evhttp_request *req, void *arg) {
    TEST_METHOD(req, EVHTTP_REQ_GET)

    SEND_REPLY_WITH_ARG(req, "farmd version: %s\r\n", VERSION)
}

static void get_barn_max_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = barn_max(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_silo_max_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = barn_max(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void field_status_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = field_status(context->db, context->field_list, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void field_harvest_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = harvest_field(context->db, context->field_list, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void plant_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    const char *query;
    GET_QUERY(req, query)

    char *post_arg = NULL;
    GET_POST_ARG_IF_NO_QUERY(query, post_arg, req)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = plant_field(context->db, &context->field_list, query, context->event_box->base, field_ready_cb, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);

    if (post_arg != NULL) {
        free(post_arg);
    }
}

static void field_ready_cb(evutil_socket_t fd, short events, void *user_data) {
    struct box_for_list_and_db *box = user_data;
    field_complete_set(box);
}

static void buy_field_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = buy_field(context->db, &context->field_list, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void buy_tree_plot_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = buy_tree_plot(context->db, &context->tree_list, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void buy_skill_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    const char *query;
    GET_QUERY(req, query)

    char *post_arg = NULL;
    GET_POST_ARG_IF_NO_QUERY(query, post_arg, req)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = buy_skill(context->db, query, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);

    if (post_arg != NULL) {
        free(post_arg);
    }
}

static void plant_tree_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    const char *query;
    GET_QUERY(req, query)

    char *post_arg = NULL;
    GET_POST_ARG_IF_NO_QUERY(query, post_arg, req)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = plant_tree(context->db, &context->tree_list, query, context->event_box->base, tree_mature_cb, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);

    if (post_arg != NULL) {
        free(post_arg);
    }
}

static void tree_mature_cb(evutil_socket_t fd, short events, void *user_data) {
    struct box_for_list_and_db *box = user_data;
    tree_mature_set(box, tree_harvest_ready_cb);
}

static void tree_harvest_ready_cb(evutil_socket_t fd, short events, void *user_data) {
    struct box_for_list_and_db *box = user_data;
    tree_complete_set(box);
}

static void tree_harvest_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = harvest_tree(context->db, context->tree_list, context->event_box->base, tree_harvest_ready_cb, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void tree_status_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = tree_status(context->db, context->tree_list, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void buy_item_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    const char *query;
    GET_QUERY(req, query)

    char *post_arg = NULL;
    GET_POST_ARG_IF_NO_QUERY(query, post_arg, req)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = buy_item(context->db, query, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);

    if (post_arg != NULL) {
        free(post_arg);
    }
}

static void sell_item_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    const char *query;
    GET_QUERY(req, query)

    char *post_arg = NULL;
    GET_POST_ARG_IF_NO_QUERY(query, post_arg, req)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = sell_item(context->db, query, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);

    if (post_arg != NULL) {
        free(post_arg);
    }
}

static void item_buy_price_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    const char *query;
    GET_QUERY(req, query)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = get_item_buy_price(context->db, query, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void item_sell_price_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    const char *query;
    GET_QUERY(req, query)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = get_item_sell_price(context->db, query, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_barn_level_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = barn_level(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_silo_level_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = silo_level(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void upgrade_barn_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = upgrade_barn(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void upgrade_silo_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = upgrade_silo(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_barn_upgrade_cost_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = barn_upgrade_cost(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_silo_upgrade_cost_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = silo_upgrade_cost(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_barn_next_level_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = barn_next_level_stats(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_silo_next_level_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = silo_next_level_stats(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void buy_grain_mill_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = buy_grain_mill(context->db, &context->grain_mill_queue, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);   
}

static void grain_mill_buy_cost_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = get_grain_mill_buy_cost(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);   
}

static void grain_mill_upgrade_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_POST)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = upgrade_grain_mill(context->db, &context->grain_mill_queue, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);   
}

static void grain_mill_upgrade_cost_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = get_grain_mill_upgrade_cost(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);   
}

static void grain_mill_next_level_stats_cb(struct evhttp_request *req, void *arg) {
    loop_context *context = arg;

    TEST_METHOD(req, EVHTTP_REQ_GET)

    int code = 0;
    struct evbuffer *returnbuffer;
    returnbuffer = get_grain_mill_next_level_stats(context->db, &code);

    evhttp_send_reply(req, code, "Client", returnbuffer);
    evbuffer_free(returnbuffer);   
}
