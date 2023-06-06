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

#include "loop.h"

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]={c, 0}
const struct timeval field_time[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]={c, 0}
const struct timeval tree_time[] = {
    TREE_CROP_TABLE
};
#undef X

void loop_run(loop_context* context) {

    // event_enable_debug_logging(EVENT_LOG_DEBUG);
    // event_set_log_callback(logging_cb);
    // event_enable_debug_mode();

    context->event_box = malloc(sizeof(events_box));

    context->event_box->base = event_base_new();

    if (!context->event_box->base) {
        abort();
    }

    context->db = NULL;
    context->tree_list = NULL;
    context->field_list = NULL;

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
        if ((context->event_box->https_socket = make_https_socket(context)) == NULL) {
            syslog(LOG_WARNING, "evhttp_accept_socket() failed");
            abort();
        }

        //make http socket if http only is false
        if (context->pre_init_info->settings->https_only == 0) {
            if ((context->event_box->http_socket = make_http_socket(context)) == NULL) {
                syslog(LOG_WARNING, "evhttp_accept_socket() failed");
                abort();
            }

            set_callbacks(context->event_box->http_base, context);
        }

        context->ssl_ctx = SSL_CTX_new(TLS_server_method());

        if (context->pre_init_info->settings->pub_key != NULL) {
            SSL_CTX_use_certificate_file(context->ssl_ctx, context->pre_init_info->settings->pub_key, SSL_FILETYPE_PEM);
        }

        if (context->pre_init_info->settings->priv_key != NULL) {
            SSL_CTX_use_PrivateKey_file(context->ssl_ctx, context->pre_init_info->settings->priv_key, SSL_FILETYPE_PEM);
        }

        SSL_CTX_check_private_key(context->ssl_ctx);

        evhttp_set_bevcb(context->event_box->https_base, make_ssl_bufferevent, context);

        set_callbacks(context->event_box->https_base, context);
    }
    else {
        if ((context->event_box->http_socket = make_http_socket(context)) == NULL) {
            syslog(LOG_WARNING, "evhttp_accept_socket() failed");
            abort();
        }

        set_callbacks(context->event_box->http_base, context);
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

static void set_callbacks(struct evhttp* base, loop_context* context) {
    evhttp_set_gencb(base, generic_http_cb, context);

    if (evhttp_set_cb(base, "/barnQuery", barn_query_cb, context)) {
        syslog(LOG_WARNING, "failed to set /barnQuery");
        abort();
    }

    if (evhttp_set_cb(base, "/siloQuery", silo_query_cb, context)) {
        syslog(LOG_WARNING, "failed to set /siloQuery");
        abort();
    }

    if (evhttp_set_cb(base, "/createSave", create_save_cb, context)) {
        syslog(LOG_WARNING, "failed to set /createSave");
        abort();
    }

    if (evhttp_set_cb(base, "/openSave", open_save_cb, context)) {
        syslog(LOG_WARNING, "failed to set /openSave");
        abort();
    }

    if (evhttp_set_cb(base, "/closeSave", close_save_cb, context)) {
        syslog(LOG_WARNING, "failed to set /closeSave");
        abort();
    }

    if (evhttp_set_cb(base, "/barnAllocation", get_barn_allocation_cb, context)) {
        syslog(LOG_WARNING, "failed to set /barnAllocation");
        abort();
    }
    
    if (evhttp_set_cb(base, "/siloAllocation", get_silo_allocation_cb, context)) {
        syslog(LOG_WARNING, "failed to set /siloAllocation");
        abort();
    }

    if (evhttp_set_cb(base, "/getMoney", get_money_cb, context)) {
        syslog(LOG_WARNING, "failed to set /getMoney");
        abort();
    }

    if (evhttp_set_cb(base, "/getLevel", get_level_cb, context)) {
        syslog(LOG_WARNING, "failed to set /getLevel");
        abort();
    }

    if (evhttp_set_cb(base, "/getXp", get_xp_cb, context)) {
        syslog(LOG_WARNING, "failed to set /getXp");
        abort();
    }

    if (evhttp_set_cb(base, "/getSkillPoints", get_skill_points_cb, context)) {
        syslog(LOG_WARNING, "failed to set /getSkillPoints");
        abort();
    }

    if (evhttp_set_cb(base, "/getSkillStatus", get_skill_status_cb, context)) {
        syslog(LOG_WARNING, "failed to set /getSkillStatus");
        abort();
    }

    if (evhttp_set_cb(base, "/field/plant", plant_cb, context)) {
        syslog(LOG_WARNING, "failed to set /field/plant");
        abort();
    }

    if (evhttp_set_cb(base, "/field/harvest", fields_harvest_cb, context)) {
        syslog(LOG_WARNING, "failed to set /field/harvest");
        abort();
    }

    if (evhttp_set_cb(base, "/field/status", fields_cb, context)) {
        syslog(LOG_WARNING, "failed to set /field/status");
        abort();
    }

    if (evhttp_set_cb(base, "/field/buy", buy_field_cb, context)) {
        syslog(LOG_WARNING, "failed to set /field/buy");
        abort();
    }

    if (evhttp_set_cb(base, "/buy/field", buy_field_cb, context)) {
        syslog(LOG_WARNING, "failed to set /buy/field");
        abort();
    }

    if (evhttp_set_cb(base, "/tree/buy", buy_tree_plot_cb, context)) {
        syslog(LOG_WARNING, "failed to set /tree/buy");
        abort();
    }

    if (evhttp_set_cb(base, "/buy/tree", buy_tree_plot_cb, context)) {
        syslog(LOG_WARNING, "failed to set /buy/tree");
        abort();
    }

    if (evhttp_set_cb(base, "/buy/skill", buy_skill_cb, context)) {
        syslog(LOG_WARNING, "failed to set /buy/skill");
        abort();
    }

    if (evhttp_set_cb(base, "/skill/buy", buy_skill_cb, context)) {
        syslog(LOG_WARNING, "failed to set /skill/buy");
        abort();
    }

    if (evhttp_set_cb(base, "/tree/plant", plant_tree_cb, context)) {
        syslog(LOG_WARNING, "failed to set /tree/plant");
        abort();
    }

    if (evhttp_set_cb(base, "/tree/harvest", tree_harvest_cb, context)) {
        syslog(LOG_WARNING, "failed to set /tree/harvest");
        abort();
    }

    if (evhttp_set_cb(base, "/tree/status", tree_status_cb, context)) {
        syslog(LOG_WARNING, "failed to set /tree/status");
        abort();
    }

    if (evhttp_set_cb(base, "/buy/item", buy_item_cb, context)) {
        syslog(LOG_WARNING, "failed to set /buy/item");
        abort();
    }

    if (evhttp_set_cb(base, "/sell/item", sell_item_cb, context)) {
        syslog(LOG_WARNING, "failed to set /sell/item");
        abort();
    }

    if (evhttp_set_cb(base, "/buy/price", item_buy_price_cb, context)) {
        syslog(LOG_WARNING, "failed to set /buy/price");
        abort();
    }

    if (evhttp_set_cb(base, "/sell/price", item_sell_price_cb, context)) {
        syslog(LOG_WARNING, "failed to set /sell/price");
        abort();
    }
}

static void sig_int_quit_term_cb(evutil_socket_t sig, short events, void* user_data) {
    loop_context* context = user_data;

    struct timeval delay = {1, 0};

    event_base_loopexit(context->event_box->base, &delay);
}

static void sighup_cb(evutil_socket_t sig, short events, void* user_data) {
    loop_context* context = user_data;

    //cache port

    int http_port = context->pre_init_info->settings->http_port;

    //reload config
    free_config_settings(context->pre_init_info->settings);

    context->pre_init_info->settings = config_parse(context->pre_init_info->config, context->pre_init_info->home, context->pre_init_info->xdg_config_home);

    //reload socket if the port has changed
    if (http_port != context->pre_init_info->settings->http_port) {
        struct evhttp_bound_socket* socket = make_http_socket(context);

        evhttp_del_accept_socket(context->event_box->http_base, context->event_box->http_socket);

        context->event_box->http_socket = socket;
    }

    //save handling
    //if from the commandline don't load.
    if (context->pre_init_info->save == NULL) {
        if (context->pre_init_info->settings->save_location != NULL) {
            close_save(context);
            open_save(context->pre_init_info->settings->save_location, context);
        }
    }

    return;
}

static void sigusr1_cb(evutil_socket_t sig, short events, void* user_data) {
    return;
}

static void sigusr2_cb(evutil_socket_t sig, short events, void* user_data) {
    return;
}

struct evhttp_bound_socket* make_http_socket(loop_context* context) {
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
    sin.sin_port = htons(context->pre_init_info->settings->http_port);
    if (bind(sock, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
        syslog(LOG_WARNING, "failed to bind socket");
        abort();
    }

    if (listen(sock, 8) < 0) {
        syslog(LOG_WARNING, "listen() failed");
        abort();
    }

    if (context->event_box->http_base == NULL) {
        context->event_box->http_base = evhttp_new(context->event_box->base);
        if (!context->event_box->http_base) {
            syslog(LOG_WARNING, "failed to make evhttp base");
            abort();
        }
    }

    return evhttp_accept_socket_with_handle(context->event_box->http_base, sock);
}

struct evhttp_bound_socket* make_https_socket(loop_context* context) {
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
    sin.sin_port = htons(context->pre_init_info->settings->https_port);
    if (bind(sock, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
        syslog(LOG_WARNING, "failed to bind socket");
        abort();
    }

    if (listen(sock, 8) < 0) {
        syslog(LOG_WARNING, "listen() failed");
        abort();
    }

    if (context->event_box->https_base == NULL) {
        context->event_box->https_base = evhttp_new(context->event_box->base);
        if (!context->event_box->https_base) {
            syslog(LOG_WARNING, "failed to make evhttp base");
            abort();
        }
    }

    return evhttp_accept_socket_with_handle(context->event_box->https_base, sock);
}

static struct bufferevent* make_ssl_bufferevent(struct event_base* base, void* user_data) {
    loop_context* context = user_data;

    struct evhttp_bound_socket* sock = context->event_box->https_socket;

    SSL* ssl = SSL_new(context->ssl_ctx);

    return bufferevent_openssl_socket_new(base, evhttp_bound_socket_get_fd(sock), ssl, BUFFEREVENT_SSL_ACCEPTING, 0);
}

static void generic_http_cb(struct evhttp_request* req, void* arg) {
    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "nothing available at %s\r\n", evhttp_request_get_uri(req));
    evhttp_send_reply(req, HTTP_NOTFOUND, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void barn_query_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    const struct evhttp_uri* uri_struct = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri_struct);

    int items = barn_query(context->db, query);

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "%s: %i\r\n", query, items);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);

    //evhttp_uri_free(uri_struct);

}

static void silo_query_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    const struct evhttp_uri* uri_struct = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri_struct);

    int items = silo_query(context->db, query);

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "%s: %i\r\n", query, items);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void create_save_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db != NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    const struct evhttp_uri* uri_struct = evhttp_request_get_evhttp_uri(req);

    const char* file_name = evhttp_uri_get_query(uri_struct);
    char* filename = NULL;

    if (file_name == NULL) {
        //check for post parameters
        struct evbuffer* inputbuffer = evhttp_request_get_input_buffer(req);
        size_t buffersize = evbuffer_get_length(inputbuffer);
        buffersize++;
        filename = malloc(buffersize);
        if (filename == NULL) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "error\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        evbuffer_copyout(inputbuffer, filename, buffersize);
        filename[buffersize - 1] = '\0';
        //check if filled
        if (strcmp(filename, "") == 0) {
            if (filename != NULL) {
                free(filename);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "no query\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        file_name = filename;
    }

    int rc = create_save(file_name, context);
    switch (rc) {
        case (1): {
            if (filename != NULL) {
                free(filename);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "failed to create new save at %s\r\n", file_name);
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
            break;
        }
        case (2): {
            if (filename != NULL) {
                free(filename);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "failed to open new save at %s for inital settings\r\n", file_name);
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
            break;
        }
        case (3): {
            if (filename != NULL) {
                free(filename);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "failed to add inital settings\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
            break;
        }
    }

    if (filename != NULL) {
        free(filename);
    }

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "new save created at %s\r\n", file_name);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void open_save_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db != NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "save already open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    const struct evhttp_uri* uri_struct = evhttp_request_get_evhttp_uri(req);

    const char* file_name = evhttp_uri_get_query(uri_struct);
    char* filename = NULL;

    if (file_name == NULL) {
        //check for post parameters
        struct evbuffer* inputbuffer = evhttp_request_get_input_buffer(req);
        size_t buffersize = evbuffer_get_length(inputbuffer);
        buffersize++;
        filename = malloc(buffersize);
        if (filename == NULL) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "error\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        evbuffer_copyout(inputbuffer, filename, buffersize);
        filename[buffersize - 1] = '\0';
        //check if filled
        if (strcmp(filename, "") == 0) {
            if (filename != NULL) {
                free(filename);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "no query\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        file_name = filename;
    }

    int rc = open_save(file_name, context);
    if (rc != 0) {
        if (filename != NULL) {
            free(filename);
        }
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to open save at %s\r\n", file_name);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (filename != NULL) {
        free(filename);
    }

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "save opened\r\n");
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void close_save_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    close_save(context);

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "save closed\r\n");
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static int open_save(const char* file_name, loop_context* context) {
    int rc = open_save_db(file_name, &context->db);
    if (rc != 0) {
        return rc;
    }

    //make fields list here
    context->field_list = make_fields_list(get_number_of_fields(context->db));

    //populate fields
    int fields = get_number_of_fields(context->db);
    if (fields > 0) {
        fields_list* list = context->field_list;
        for (int i = 0; i < fields; i++) {
            const char* field_type = get_field_type(context->db, i);
            list->type = field_crop_string_to_enum(field_type);
            free((char*) field_type);

            if (list->type != NONE_FIELD) {

                if (get_field_completion(context->db, list->field_number) == 0) {
                    time_t now = time(NULL);
                    time_t time_from_db = get_field_time(context->db, list->field_number);

                    if (time_from_db > now) {
                        //make an event as it has not finshed
                        struct timeval tv;
                        tv.tv_sec = time_from_db - now;
                        tv.tv_usec = 0;

                        struct box_for_list_and_db* box = malloc(sizeof(struct box_for_list_and_db));
                        box->list = list;
                        box->db = context->db;
                        list->event = event_new(context->event_box->base, -1, 0, field_ready_cb, box);

                        event_add(list->event, &tv);
                    }
                    else {
                        //if it is not less then it is equal or greater
                        list->completion = 1;
                        set_field_completion(context->db, list->field_number, 1);
                    }
                }
                else {
                    list->completion = 1;
                }
            }
            

            list = list->next;
        }
    }

    context->tree_list = make_trees_list(get_number_of_tree_plots(context->db));

    //populate trees
    int trees = get_number_of_tree_plots(context->db);
    if (trees > 0) {
        trees_list* list = context->tree_list;
        for (int i = 0; i < trees; i++) {
            const char* tree_type = get_tree_type(context->db, i);
            list->type = tree_crop_string_to_enum(tree_type);
            free((char*) tree_type);
            //events here later

            if (list->type != NONE_TREE) {
                
                if (get_tree_completion(context->db, list->tree_number) == 0) {
                    time_t now = time(NULL);
                    time_t time_from_db = get_tree_time(context->db, list->tree_number);

                    if (time_from_db > now) {
                        //make an event as it has not finshed
                        struct timeval tv;
                        tv.tv_sec = time_from_db - now;
                        tv.tv_usec = 0;

                        struct box_for_list_and_db* box = malloc(sizeof(struct box_for_list_and_db));
                        box->list = list;
                        box->db = context->db;
                        list->event = event_new(context->event_box->base, -1, 0, tree_harvest_ready_cb, box);

                        event_add(list->event, &tv);
                    }
                    else {
                        //if it is not less then it is equal or greater
                        list->completion = 1;
                        set_tree_completion(context->db, list->tree_number, 1);
                    }
                }
                else {
                    list->completion = 1;
                }
            }

            list = list->next;
        }
    }

    return 0;
}

static int create_save(const char* file_name, loop_context* context) {
    int rc = create_save_db(file_name);
    if (rc != 0) {
        return 1;
    }

    rc = open_save(file_name, context);
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

static int close_save(loop_context* context) {
    close_save_db(context->db);
    context->db = NULL;

    //free fields
    if (context->field_list != NULL) {
        fields_list* field_list = context->field_list;
        while (field_list != NULL) {
            if (field_list->event != NULL) {
                event_del(field_list->event);
                void* temp;
                if ((temp = event_get_callback_arg(field_list->event)) != NULL) {
                    free(temp);
                }
                event_free(field_list->event);
            }
            fields_list* temp = field_list;
            field_list = field_list->next;
            free(temp);
        }
    }

    //free trees
    if (context->tree_list != NULL) {
        trees_list* tree_list = context->tree_list;
        while (tree_list->event != NULL) {
            event_del(tree_list->event);
                void* temp;
                if ((temp = event_get_callback_arg(tree_list->event)) != NULL) {
                    free(temp);
                }
            event_free(tree_list->event);
        }
        trees_list* temp = tree_list;
        tree_list = tree_list->next;
        free(temp);
    }

    return 0;
}

static void get_barn_allocation_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    int max = get_barn_max(context->db);

    int used = get_barn_allocation(context->db);

    float allocation = (float) used / (float) max;

    allocation = allocation * 100;

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "barn used: %.2f%%\r\n", allocation);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_silo_allocation_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    int max = get_silo_max(context->db);

    int used = get_silo_allocation(context->db);

    float allocation = (float) used / (float) max;

    allocation = allocation * 100;

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "silo used: %.2f%%\r\n", allocation);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_money_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    int money = get_money(context->db);

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "money: %d\r\n", money);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_level_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    int level = get_level(context->db);

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "level: %d\r\n", level);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_xp_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    int xp = get_xp(context->db);

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "xp: %d\r\n", xp);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_skill_points_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    int skill_points = get_skill_points(context->db);

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "Skill points: %d\r\n", skill_points);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void get_skill_status_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    const struct evhttp_uri* uri_struct = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri_struct);

    int skill_status = get_skill_status(context->db, query);

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "Skill status: %s %d\r\n", query, skill_status);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void fields_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (get_number_of_fields(context->db) == 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (context->field_list == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    fields_list* list = context->field_list;

    struct evbuffer* returnbuffer = evbuffer_new();

    //check for races
    do {
        char* complete;
        if (list->completion == 1) {
            complete = "ready";
            //make sure its right
            set_field_completion(context->db, list->field_number, 1);
        }
        else if (get_field_completion(context->db, list->field_number) == 1) {
            complete = "ready";
        }
        else {
            complete = "not ready";
        }
        evbuffer_add_printf(returnbuffer, "field%d: %s %s\r\n", list->field_number, field_crop_enum_to_string(list->type), complete);
        list = list->next;
    } while (list != NULL);

    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void fields_harvest_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (get_number_of_fields(context->db) == 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (context->field_list == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    fields_list* list = context->field_list;

    do {
        if (list->completion == 1 && list->type != NONE_FIELD) {
            enum storage storage_place = get_storage_type_field(list->type);

            if (storage_place == SILO) {

                if (get_silo_allocation(context->db) < (get_silo_max(context->db) - 1)) {
                    if (add_item_to_silo(context->db, field_crop_enum_to_string(list->type), UNLOCKED) != 0) {
                        //INSERT OR IGNORE shouldnt cause an issue or any slowdowns I hope
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "silo error\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                    if (update_silo(context->db, field_crop_enum_to_string(list->type), 2) != 0) {
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "silo error\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                }
                else {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "could not harvest field%d due to silo size\r\n", list->field_number);
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    return;
                }
            }
            else if (storage_place == BARN) {

                if (get_barn_allocation(context->db) < (get_barn_max(context->db) - 1)) {
                    if (add_item_to_silo(context->db, field_crop_enum_to_string(list->type), UNLOCKED) != 0) {
                        //INSERT OR IGNORE shouldnt cause an issue or any slowdowns I hope
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "barn error\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                    if (update_barn(context->db, field_crop_enum_to_string(list->type), 2) != 0) {
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "barn error\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                }
                else {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "could not harvest field%d due to barn size\r\n", list->field_number);
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    return;
                }
            }
            else {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "could not harvest field%d due to not being implemented\r\n", list->field_number);
                evhttp_send_reply(req, HTTP_NOTIMPLEMENTED, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                return;
            }

            list->type = NONE_FIELD;
            list->completion = 0;
            remove_field(context->db, list->field_number);
            //reset the complete flag to false
            set_field_completion(context->db, list->field_number, 0);
            //clear the field time as it had now been harvested
            clear_field_time(context->db, list->field_number);
            update_meta(context->db, 2, "xp");
            xp_check(context->db);
        }

        list = list->next;
    } while (list != NULL);

    evhttp_send_reply(req, HTTP_OK, "Client", NULL);
}

static void plant_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (get_number_of_fields(context->db) == 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (context->field_list == NULL) {
        context->field_list = make_fields_list(get_number_of_fields(context->db));
        if (context->field_list == NULL) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "could not make fields\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
         for (int i = 0; i < get_number_of_fields(context->db); i++) {
            if (add_field(context->db, i) != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding field to db\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error adding field to db");
                return;
            }
        }
    }

    int current = 0;
    if ((current = get_number_of_fields_list(context->field_list)) < get_number_of_fields(context->db)) {
        if (amend_fields_list(context->field_list, get_number_of_fields(context->db)) != 0) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "could not amend fields\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        for (int i = current; i < get_number_of_fields(context->db); i++) {
            if (add_field(context->db, i) != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding field to db\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error adding field to db");
                return;
            }
        }
    }

    const struct evhttp_uri* uri_struct = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri_struct);
    char* post_arg = NULL;

    if (query == NULL) {
        //check for post parameters
        struct evbuffer* inputbuffer = evhttp_request_get_input_buffer(req);
        size_t buffersize = evbuffer_get_length(inputbuffer);
        buffersize++;
        post_arg = malloc(buffersize);
        if (post_arg == NULL) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "error\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        evbuffer_copyout(inputbuffer, post_arg, buffersize);
        post_arg[buffersize - 1] = '\0';
        //check if filled
        if (strcmp(post_arg, "") == 0) {
            if (post_arg != NULL) {
                free(post_arg);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "no query\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        query = post_arg;
    }

    enum field_crop type;

    type = field_crop_string_to_enum(query);
    if (type == NONE_FIELD) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "%s is not a correct query\r\n", query);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        if (post_arg != NULL) {
            free(post_arg);
        }
        return;
    }

    if (post_arg != NULL) {
        free(post_arg);
    }

    const char* sanitized_string = field_crop_enum_to_string(type);

    if (get_skill_status(context->db, sanitized_string) == 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "currently do not own %s skill\r\n", sanitized_string);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    fields_list* list = context->field_list;

    int planted = 0;

    do {
        if (list->type == NONE_FIELD) {
            if (list->event == NULL) {
                struct box_for_list_and_db* box = malloc(sizeof(struct box_for_list_and_db));
                box->list = list;
                box->db = context->db;

                list->event = event_new(context->event_box->base, -1, 0, field_ready_cb, box);
                if (list->event == NULL) {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "error making event\r\n");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    syslog(LOG_WARNING, "error making event for fields");
                    return;
                }
            }

            int price = field_crop_buy_cost(type);

            enum storage store = get_storage_type_field(type);

            //consume crops or cash
            if (store == SILO) {
                if (silo_query(context->db, sanitized_string) > 0) {
                    if (update_silo(context->db, sanitized_string, -1) != 0) {
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "could not update silo\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                }
            }
            else if (store == BARN) {
                if (barn_query(context->db, sanitized_string) > 0) {
                    if (update_barn(context->db, sanitized_string, -1) != 0) {
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "could not update barn\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                }
            }
            else if (get_money(context->db) > price) {
                if (update_meta(context->db, (-1 * price), "Money") != 0) {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "could not update money\r\n");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    return;
                }
            }
            else {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "could not plant or buy\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                return;
            }

            list->type = type;
            set_field_type(context->db, list->field_number, field_crop_enum_to_string(type));
            //set field completion
            set_field_completion(context->db, list->field_number, 0);
            //set time it should complete
            set_field_time(context->db, list->field_number, field_time[type].tv_sec);

            const struct timeval* tv = &field_time[type];
            int rc = event_add(list->event, tv);
            if (rc != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding event\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error adding field event");
                return;
            }
            planted++;
        } 
        list = list->next;
    } while (list != NULL);


    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "planted: %d\r\n", planted);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void field_ready_cb(evutil_socket_t fd, short events, void* user_data) {
    struct box_for_list_and_db* box = user_data;

    fields_list* list = box->list;

    list->completion = 1;

    set_field_completion(box->db, list->field_number, 1);

    return;
}

static void xp_check(sqlite3* db) {
    int level = get_level(db);
    level--;
    int xp_needed = pow(2, level) * 10;
    //xp needed for level 2 is 10
    //level 3 is 20
    //level 4 is 40
    //level 5 is 80
    //level 6 is 160
    int xp = get_xp(db);
    if (xp >= xp_needed) {
        if (level_up(db, xp_needed) != 0) {
            syslog(LOG_WARNING, "error leveling up");
            return;
        }
        if (update_meta(db, 2, "SkillPoints") != 0) {
            syslog(LOG_WARNING, "error adding skill points");
            return;
        }
    }
}

static void buy_field_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    //check skill tree first
    int current = get_number_of_fields(context->db);
    int skill_level = get_skill_status(context->db, "Fields");
    if (current < (skill_level * 3)) {
        //price is 2^current fields for next
        if (get_money(context->db) > pow(2, current)) {
            if (update_meta(context->db, (-1 * (pow(2, current))), "Money") == 0) {
                if (update_meta(context->db, 1, "Fields") != 0) {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "error adding field\r\n");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    syslog(LOG_WARNING, "error adding field");
                    return;
                }
            }
            else {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error subtracting money\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                return;
            }
        }
        else {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "not enough money to buy field\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
    }
    else {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "not high enough skill level to buy field\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (current == 0) {
        context->field_list = make_fields_list(get_number_of_fields(context->db));

        //get the sql in shape
        int fields = get_number_of_fields(context->db);
        fields_list* list = context->field_list;
        for (int i = 0; i < fields; i++) {
            if (add_field(context->db, list->field_number) != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding field to db\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error adding field to db");
                return;
            }
        }
    }
    else {
        if (amend_fields_list(context->field_list, get_number_of_fields(context->db)) != 0) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "error amending field list\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            syslog(LOG_WARNING, "error amending field list");
            return;
        }
        for (int i = current; i < get_number_of_fields(context->db); i++) {
            if (add_field(context->db, i) != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding field to db\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error adding field to db");
                return;
            }
        }
    }

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "sucessfully bought field\r\n");
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
    return;
}

static void buy_tree_plot_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    int current = get_number_of_tree_plots(context->db);
    int skill_level = get_skill_status(context->db, "TreePlots");
    if (current < skill_level) {
        //price is 2^current + 1 plots for next
        if (get_money(context->db) > pow(2, current + 1)) {
            if (update_meta(context->db, (-1 * (pow(2, current + 1))), "Money") == 0) {
                if (update_meta(context->db, 1, "TreePlots") != 0) {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "error adding tree plot\r\n");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    syslog(LOG_WARNING, "error adding tree plot");
                    return;
                }
            }
            else {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error subtracting money\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                return;
            }
        }
        else {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "not enough money to buy tree plot\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
    }
    else {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "not high enough skill level to buy this\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (current == 0) {
        context->tree_list = make_trees_list(get_number_of_tree_plots(context->db));
        for (int i = 0; i < get_number_of_tree_plots(context->db); i++) {
            if (add_tree(context->db, i) != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding tree plot\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                return;
            }
        }
    }
    else {
        if (amend_trees_list(context->tree_list, get_number_of_tree_plots(context->db)) != 0) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "error amending tree list\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            syslog(LOG_WARNING, "error amending tree list");
            return;
        }
        for (int i = current; i < get_number_of_tree_plots(context->db); i++) {
            if (add_tree(context->db, i) != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding tree plot\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                return;
            }
        }
    }

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "sucessfully bought tree plot\r\n");
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
    return;
}

static void buy_skill_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri);
    char* post_arg = NULL;

    if (query == NULL) {
        //check for post parameters
        struct evbuffer* inputbuffer = evhttp_request_get_input_buffer(req);
        size_t buffersize = evbuffer_get_length(inputbuffer);
        buffersize++;
        post_arg = malloc(buffersize);
        if (post_arg == NULL) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "error\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        evbuffer_copyout(inputbuffer, post_arg, buffersize);
        post_arg[buffersize - 1] = '\0';
        //check if filled
        if (strcmp(post_arg, "") == 0) {
            if (post_arg != NULL) {
                free(post_arg);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "no query\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        query = post_arg;
    }

    if (strcasecmp(query, "Fields") == 0) {
        //limit is 1 per level
        if (get_skill_status(context->db, "Fields") >= get_level(context->db)) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "failed to buy %s: limit is 1 per level\r\n", query);
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            if (post_arg != NULL) {
                free(post_arg);
            }
            return;
        }
    }
    else if (strcasecmp(query, "TreePlots") == 0) {
        //limit is 1 per level
        if (get_skill_status(context->db, "TreePlots") >= get_level(context->db)) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "failed to buy %s: limit is 1 per level\r\n", query);
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            if (post_arg != NULL) {
                free(post_arg);
            }
            return;
        }
    }
    else if (get_skill_status(context->db, query) > 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "already own skill: %s\r\n", query);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        if (post_arg != NULL) {
            free(post_arg);
        }
        return;
    }

    //hash table here for depenacny checking
    const char* reason = skill_dep_check(context->db, query);
    if (reason != NULL) {
        if (post_arg != NULL) {
            free(post_arg);
        }
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "%s\r\n", reason);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    int skill_points = get_skill_points(context->db);
    if (skill_points < 1) {
        if (post_arg != NULL) {
            free(post_arg);
        }
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "not enough skill points\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }
    if (update_meta(context->db, -1, "SkillPoints") == 0) {
        if (update_skill_tree(context->db, query) != 0) {
            if (post_arg != NULL) {
                free(post_arg);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "error adding skill\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
    }
    else {
        if (post_arg != NULL) {
            free(post_arg);
        }
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "error subtracting skill points\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return; 
    }

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "sucessfully bought skill: %s\r\n", query);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
    if (post_arg != NULL) {
        free(post_arg);
    }
    return;
}

static void plant_tree_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (get_number_of_tree_plots(context->db) == 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no tree plots\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (context->tree_list == NULL) {
        context->tree_list = make_trees_list(get_number_of_tree_plots(context->db));
        if (context->tree_list == NULL) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "could not make trees\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        for (int i = 0; i < get_number_of_tree_plots(context->db); i++) {
            if (add_tree(context->db, i) != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding tree to db\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error adding tree to db");
                return;
            }
        }
    }

    int current;
    if ((current = get_number_of_trees_list(context->tree_list)) < get_number_of_tree_plots(context->db)) {
        if (amend_trees_list(context->tree_list, get_number_of_tree_plots(context->db)) != 0) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "could not amend trees\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        for (int i = current; i < get_number_of_tree_plots(context->db); i++) {
            if (add_tree(context->db, i) != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding tree to db\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error adding tree to db");
                return;
            }
        }
    }

    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri);
    char* post_arg = NULL;

    if (query == NULL) {
        //check for post parameters
        struct evbuffer* inputbuffer = evhttp_request_get_input_buffer(req);
        size_t buffersize = evbuffer_get_length(inputbuffer);
        buffersize++;
        post_arg = malloc(buffersize);
        if (post_arg == NULL) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "error\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        evbuffer_copyout(inputbuffer, post_arg, buffersize);
        post_arg[buffersize - 1] = '\0';
        //check if filled
        if (strcmp(post_arg, "") == 0) {
            if (post_arg != NULL) {
                free(post_arg);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "no query\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        query = post_arg;
    }

    enum tree_crop type;

    type = tree_crop_string_to_enum(query);

    if (type == NONE_TREE) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "%s is not a correct query\r\n", query);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        if (post_arg != NULL) {
            free(post_arg);
        }
        return;
    }

    if (post_arg != NULL) {
        free(post_arg);
    }

    const char* sanitized_string = tree_crop_enum_to_string(type);

    if (get_skill_status(context->db, sanitized_string) == 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "currently do not own %s skill\r\n", sanitized_string);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    //Plant tree plot
    trees_list* list = context->tree_list;

    int tree_plot = 0;

    //so only one tree can be planted at a time
    int done;

    do {
        if (list->type == NONE_TREE) {
            //plant a tree
            //check if event is clear
            if (list->event == NULL) {
                struct box_for_list_and_db* box = malloc(sizeof(struct box_for_list_and_db));
                box->list = list;
                box->db = context->db;

                list->event = event_new(context->event_box->base, -1, 0, tree_harvest_ready_cb, box);
                if (list->event == NULL) {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "error making event\r\n");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    syslog(LOG_WARNING, "error making event for trees");
                    return;
                }

            }

            int price = tree_crop_buy_cost(type);

            enum storage storage_place = get_storage_type_tree(type);

            //consume crops or cash
            if (storage_place == BARN) {
                if (barn_query(context->db, sanitized_string) > 0) {
                    if (update_barn(context->db, sanitized_string, -1) != 0) {
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "could not update barn\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                }
            }
            else if (storage_place == SILO) {
                if (silo_query(context->db, sanitized_string) > 0) {
                    if (update_silo(context->db, sanitized_string, -1) != 0) {
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "could not update silo\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                }
            }
            else if (get_money(context->db) > price) {
                if (update_meta(context->db, (-1 * price), "Money") != 0) {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "could not update money\r\n");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    return;
                }
            }
            else {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "could not plant or buy\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                return;
            }

            //add to trees table
            set_tree_type(context->db, list->tree_number, sanitized_string);
            set_tree_completion(context->db, list->tree_number, 0);

            //set the completetion time
            set_tree_time(context->db, list->tree_number, tree_time[list->type].tv_sec);

            //set the tree timer
            const struct timeval* tv = &tree_time[list->type];
            int rc = event_add(list->event, tv);
            if (rc != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding event\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error adding tree event");
                return;
            }

            list->type = type;
            tree_plot = list->tree_number;

            done = 1;
        }

       list = list->next;

    } while(list != NULL && done == 0);

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "planted tree plot: %d\r\n", tree_plot);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void tree_harvest_ready_cb(evutil_socket_t fd, short events, void* user_data) {
    struct box_for_list_and_db* box = user_data;
    trees_list* list = box->list;

    list->completion = 1;

    set_tree_completion(box->db, list->tree_number, 1);

    return;
}

static void tree_harvest_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (get_number_of_tree_plots(context->db) == 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no trees\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (context->tree_list == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no tree\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    trees_list* list = context->tree_list;

    do {
        if (list->completion == 1 && list->type != NONE_TREE) {
            enum storage storage_place = get_storage_type_tree(list->type);

            if (storage_place == BARN) {
                if (get_barn_allocation(context->db) < (get_barn_max(context->db)) - 1) {
                    if (add_item_to_barn(context->db, tree_crop_enum_to_string(list->type), UNLOCKED) != 0) {
                        //INSERT OR IGNORE shouldnt cause an issue or any slowdowns I hope
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "barn error\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                    if (update_barn(context->db, tree_crop_enum_to_string(list->type), 2) != 0) {
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "barn error\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                }
                else {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "could not harvest tree%d due to barn size\r\n", list->tree_number);
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    return;
                }
            }
            else if (storage_place == SILO) {
                if (get_silo_allocation(context->db) < (get_silo_max(context->db)) - 1) {
                    if (add_item_to_silo(context->db, tree_crop_enum_to_string(list->type), UNLOCKED) != 0) {
                        //INSERT OR IGNORE shouldnt cause an issue or any slowdowns I hope
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "silo error\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                    if (update_silo(context->db, tree_crop_enum_to_string(list->type), 2) != 0) {
                        struct evbuffer* returnbuffer = evbuffer_new();
                        evbuffer_add_printf(returnbuffer, "silo error\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        return;
                    }
                }
                else {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "could not harvest tree%d due to silo size\r\n", list->tree_number);
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    return;
                }
            }
            else {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "could not harvest tree%d due to not being implemented\r\n", list->tree_number);
                evhttp_send_reply(req, HTTP_NOTIMPLEMENTED, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                return;
            }
            
            list->completion = 0;

            //update the db
            set_tree_completion(context->db, list->tree_number, 0);
            set_tree_time(context->db, list->tree_number, tree_time[list->type].tv_sec);

            const struct timeval* tv = &tree_time[list->type];
            int rc = event_add(list->event, tv);
            if (rc != 0) {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "error adding event\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error adding tree event");
                return;
            }

            update_meta(context->db, 2, "xp");
            xp_check(context->db);
        }
        list = list->next;
    } while (list != NULL);

    evhttp_send_reply(req, HTTP_OK, "Client", NULL);
}

static void tree_status_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (get_number_of_tree_plots(context->db) == 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no tree plots\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (context->tree_list == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no trees\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    trees_list* list = context->tree_list;

    struct evbuffer* returnbuffer = evbuffer_new();

    //check for races
    do {
        char* complete;
        if (list->completion == 1) {
            complete = "ready";
            set_tree_completion(context->db, list->tree_number, 1);
        }
        else if (get_tree_completion(context->db, list->tree_number) == 1) {
            complete = "ready";
        }
        else {
            complete = "not ready";
        }
        evbuffer_add_printf(returnbuffer, "tree%d: %s %s\r\n", list->tree_number, tree_crop_enum_to_string(list->type), complete);
        list = list->next;
    } while (list != NULL);

    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void buy_item_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri);
    char* post_arg = NULL;

    if (query == NULL) {
        //check for post parameters
        struct evbuffer* inputbuffer = evhttp_request_get_input_buffer(req);
        size_t buffersize = evbuffer_get_length(inputbuffer);
        buffersize++;
        post_arg = malloc(buffersize);
        if (post_arg == NULL) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "error\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        evbuffer_copyout(inputbuffer, post_arg, buffersize);
        post_arg[buffersize - 1] = '\0';
        //check if filled
        if (strcmp(post_arg, "") == 0) {
            if (post_arg != NULL) {
                free(post_arg);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "no query\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        query = post_arg;
    }

    if (post_arg != NULL) {
        free(post_arg);
    }

    enum item_type type =  get_product_type_string(query);

    int item_price = 0;
    enum storage storage_place;
    const char* sanitized_string;

    switch (type) {
        case NONE_PRODUCT:
            item_price = 0;
            break;
        case FIELD_PRODUCT: {
            enum field_crop item = field_crop_string_to_enum(query);
            item_price = field_crop_buy_cost(item);
            storage_place = get_storage_type_field(item);
            sanitized_string = field_crop_enum_to_string(item);
            break;
        }
        case TREE_PRODUCT: {
            enum tree_crop item = tree_crop_string_to_enum(query);
            item_price = tree_crop_buy_cost(item);
            storage_place = get_storage_type_tree(item);
            sanitized_string = tree_crop_enum_to_string(item);
            break;
        }
        case OTHER_PRODUCT:
            //nop
            item_price = 0;
            break;
        default:
            item_price = 0;
            break;
    }

    struct evbuffer* returnbuffer = evbuffer_new();

    if (item_price == 0) {
        evbuffer_add_printf(returnbuffer, "%s: is not valid\r\n", query);
        evhttp_send_reply(req, HTTP_BADREQUEST, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    //db work
    //update money
    if (get_money(context->db) > item_price) {
        if (update_meta(context->db, (-1 * item_price), "Money")) {
            evbuffer_add_printf(returnbuffer, "error updating money\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            syslog(LOG_WARNING, "error updating money");
            return;
        }
    }
    else {
        evbuffer_add_printf(returnbuffer, "not enough money to buy item\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (storage_place == BARN) {
        //check allocation
        if (get_barn_allocation(context->db) < get_barn_max(context->db)) {
            //check if item is there
            if (barn_query(context->db, sanitized_string) != -1) {
                //remove item
                if (update_barn(context->db, sanitized_string, 1) != 0) {
                    evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    syslog(LOG_WARNING, "error updating barn");
                    return;
                }
            }
            else {
                if (get_skill_status(context->db, sanitized_string) == 0) {
                    if (add_item_to_barn(context->db, sanitized_string, LOCKED) != 0) {
                        evbuffer_add_printf(returnbuffer, "error adding item to barn\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        syslog(LOG_WARNING, "error adding item to barn");
                        return;
                    }
                    if (update_barn(context->db, sanitized_string, 1) != 0) {
                        evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        syslog(LOG_WARNING, "error updating barn");
                        return;
                    }
                }
                else {
                    if (add_item_to_barn(context->db, sanitized_string, UNLOCKED) != 0) {
                        evbuffer_add_printf(returnbuffer, "error adding item to barn\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        syslog(LOG_WARNING, "error adding item to barn");
                        return;
                    }
                    if (update_barn(context->db, sanitized_string, 1) != 0) {
                        evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        syslog(LOG_WARNING, "error updating barn");
                        return;
                    }
                }
            }
        }
        else {
            evbuffer_add_printf(returnbuffer, "error buying due to barn size\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
    }
    else if (storage_place == SILO) {
        //check allocation
        if (get_silo_allocation(context->db) < get_silo_max(context->db)) {
            //check if item is there
            if (silo_query(context->db, sanitized_string) != -1) {
                //add item
                if (update_silo(context->db, sanitized_string, 1) != 0) {
                    evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    syslog(LOG_WARNING, "error updating silo");
                    return;
                }
            }
            else {
                if (get_skill_status(context->db, sanitized_string) == 0) {
                    if (add_item_to_silo(context->db, sanitized_string, LOCKED) != 0) {
                        evbuffer_add_printf(returnbuffer, "error adding item to silo\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        syslog(LOG_WARNING, "error adding item to silo");
                        return;
                    }
                    if (update_silo(context->db, sanitized_string, 1) != 0) {
                        evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        syslog(LOG_WARNING, "error updating silo");
                        return;
                    }
                }
                else {
                    if (add_item_to_silo(context->db, sanitized_string, UNLOCKED) != 0) {
                        evbuffer_add_printf(returnbuffer, "error adding item to silo\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        syslog(LOG_WARNING, "error adding item to silo");
                        return;
                    }
                    if (update_silo(context->db, sanitized_string, 1) != 0) {
                        evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
                        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                        evbuffer_free(returnbuffer);
                        syslog(LOG_WARNING, "error updating silo");
                        return;
                    }
                }
            }
        }
        else {
            evbuffer_add_printf(returnbuffer, "error buying due to silo size\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
    }
    else {
        evhttp_send_reply(req, HTTP_NOTIMPLEMENTED, "Client", NULL);
        evbuffer_free(returnbuffer);
        return;
    }

    evbuffer_add_printf(returnbuffer, "sucessfully bought: %s\r\n", sanitized_string);

    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void sell_item_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_POST) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri);
    char* post_arg = NULL;

    if (query == NULL) {
        //check for post parameters
        struct evbuffer* inputbuffer = evhttp_request_get_input_buffer(req);
        size_t buffersize = evbuffer_get_length(inputbuffer);
        buffersize++;
        post_arg = malloc(buffersize);
        if (post_arg == NULL) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "error\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        evbuffer_copyout(inputbuffer, post_arg, buffersize);
        post_arg[buffersize - 1] = '\0';
        //check if filled
        if (strcmp(post_arg, "") == 0) {
            if (post_arg != NULL) {
                free(post_arg);
            }
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "no query\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
        query = post_arg;
    }

    if (post_arg != NULL) {
        free(post_arg);
    }

    enum item_type type =  get_product_type_string(query);

    int item_price = 0;
    enum storage storage_place;
    const char* sanitized_string;

    switch (type) {
        case NONE_PRODUCT:
            item_price = 0;
            break;
        case FIELD_PRODUCT: {
            enum field_crop item = field_crop_string_to_enum(query);
            item_price = field_crop_sell_cost(item);
            storage_place = get_storage_type_field(item);
            sanitized_string = field_crop_enum_to_string(item);
            break;
        }
        case TREE_PRODUCT: {
            enum tree_crop item = tree_crop_string_to_enum(query);
            item_price = tree_crop_sell_cost(item);
            storage_place = get_storage_type_tree(item);
            sanitized_string = tree_crop_enum_to_string(item);
            break;
        }
        case OTHER_PRODUCT:
            //nop
            item_price = 0;
            break;
        default:
            item_price = 0;
            break;
    }

    struct evbuffer* returnbuffer = evbuffer_new();

    if (item_price == 0) {
        evbuffer_add_printf(returnbuffer, "%s: is not valid\r\n", query);
        evhttp_send_reply(req, HTTP_BADREQUEST, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    //db work
    if (storage_place == BARN) {
        //check if item is there
        if (barn_query(context->db, sanitized_string) > 0) {
            //remove item
            if (update_barn(context->db, sanitized_string, -1) != 0) {
                evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error updating barn");
                return;
            }
        }
        else {
            evbuffer_add_printf(returnbuffer, "not enough %s in barn\r\n", sanitized_string);
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
    }
    else if (storage_place == SILO) {
        //check if item is there
        if (silo_query(context->db, sanitized_string) > 0) {
            //remove item
            if (update_silo(context->db, sanitized_string, -1) != 0) {
                evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                syslog(LOG_WARNING, "error updating silo");
                return;
            }
        }
        else {
            evbuffer_add_printf(returnbuffer, "not enough %s in silo\r\n", sanitized_string);
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
    }
    else {
        evhttp_send_reply(req, HTTP_NOTIMPLEMENTED, "Client", NULL);
        evbuffer_free(returnbuffer);
        return;
    }

    //update money
    if (update_meta(context->db, item_price, "Money")) {
        evbuffer_add_printf(returnbuffer, "error updating money\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        syslog(LOG_WARNING, "error updating money");
        return;
    }

    evbuffer_add_printf(returnbuffer, "sucessfully sold: %s for %d\r\n", sanitized_string, item_price);

    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void item_buy_price_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri);

    enum item_type type =  get_product_type_string(query);

    struct evbuffer* returnbuffer = evbuffer_new();

    switch (type) {
        case NONE_PRODUCT:
            evbuffer_add_printf(returnbuffer, "%s: is not a valid item\r\n", query);
            break;
        case FIELD_PRODUCT: {
            enum field_crop item = field_crop_string_to_enum(query);
            evbuffer_add_printf(returnbuffer, "%s: %d\r\n", query, field_crop_buy_cost(item));
            break;
        }
        case TREE_PRODUCT: {
            enum tree_crop item = tree_crop_string_to_enum(query);
            evbuffer_add_printf(returnbuffer, "%s: %d\r\n", query, tree_crop_buy_cost(item));
            break;
        }
        case OTHER_PRODUCT:
            //nop
            evbuffer_add_printf(returnbuffer, "%s: not yet implemented\r\n", query);
            break;
        default:
            evbuffer_add_printf(returnbuffer, "%s: is not a valid item\r\n", query);
            break;
    }

    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void item_sell_price_cb(struct evhttp_request* req, void* arg) {
    loop_context* context = arg;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", NULL);
        return;
    }

    if (context->db == NULL) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri);

    enum item_type type =  get_product_type_string(query);

    struct evbuffer* returnbuffer = evbuffer_new();

    switch (type) {
        case NONE_PRODUCT:
            evbuffer_add_printf(returnbuffer, "%s: is not a valid item\r\n", query);
            break;
        case FIELD_PRODUCT: {
            enum field_crop item = field_crop_string_to_enum(query);
            evbuffer_add_printf(returnbuffer, "%s: %d\r\n", query, field_crop_sell_cost(item));
            break;
        }
        case TREE_PRODUCT: {
            enum tree_crop item = tree_crop_string_to_enum(query);
            evbuffer_add_printf(returnbuffer, "%s: %d\r\n", query, tree_crop_sell_cost(item));
            break;
        }
        case OTHER_PRODUCT:
            //nop
            evbuffer_add_printf(returnbuffer, "%s: not yet implemented\r\n", query);
            break;
        default:
            evbuffer_add_printf(returnbuffer, "%s: is not a valid item\r\n", query);
            break;
    }

    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}
