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

#include "loop.h"

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
    sin.sin_port = htons(8080);
    if (bind(sock, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
        syslog(LOG_WARNING, "failed to bind socket");
        abort();
    }

    if (listen(sock, 8) < 0) {
        syslog(LOG_WARNING, "listen() failed");
        abort();
    }

    context->event_box->http_base = evhttp_new(context->event_box->base);

    if (evhttp_accept_socket(context->event_box->http_base, sock) == -1) {
        syslog(LOG_WARNING, "evhttp_accept_socket() failed");
        abort();
    }

    evhttp_set_gencb(context->event_box->http_base, generic_http_cb, context);

    if (evhttp_set_cb(context->event_box->http_base, "/barnQuery", barn_query_cb, context)) {
        syslog(LOG_WARNING, "failed to set /barnQuery");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/siloQuery", silo_query_cb, context)) {
        syslog(LOG_WARNING, "failed to set /siloQuery");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/createSave", create_save_cb, context)) {
        syslog(LOG_WARNING, "failed to set /createSave");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/openSave", open_save_cb, context)) {
        syslog(LOG_WARNING, "failed to set /openSave");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/closeSave", close_save_cb, context)) {
        syslog(LOG_WARNING, "failed to set /closeSave");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/barnAllocation", get_barn_allocation_cb, context)) {
        syslog(LOG_WARNING, "failed to set /barnAllocation");
        abort();
    }
    
    if (evhttp_set_cb(context->event_box->http_base, "/siloAllocation", get_silo_allocation_cb, context)) {
        syslog(LOG_WARNING, "failed to set /siloAllocation");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/getMoney", get_money_cb, context)) {
        syslog(LOG_WARNING, "failed to set /getMoney");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/getLevel", get_level_cb, context)) {
        syslog(LOG_WARNING, "failed to set /getLevel");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/getXp", get_xp_cb, context)) {
        syslog(LOG_WARNING, "failed to set /getXp");
        abort();
    }

    /*
    Message message = new Message("/remote/object/path", "MethodName", arg1, arg2);
          Connection connection = getBusConnection();
          connection.send(message);
          Message reply = connection.waitForReply(message);
          if (reply.isError()) {
             
          } else {
             Object returnValue = reply.getReturnValue();
          }


    set up signals that can be sent. Use for stuff like Harvest is ready and new contract is in.
    */


    //run loop
    event_base_loop(context->event_box->base, EVLOOP_NO_EXIT_ON_EMPTY);

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

    event_base_free(context->event_box->base);

    free(context->event_box);

    return;
}

static void sig_int_quit_term_cb(evutil_socket_t sig, short events, void* user_data) {
    loop_context* context = (loop_context*) user_data;

    struct timeval delay = {1, 0};

    event_base_loopexit(context->event_box->base, &delay);
}

static void sighup_cb(evutil_socket_t sig, short events, void* user_data) {
    return;
}

static void sigusr1_cb(evutil_socket_t sig, short events, void* user_data) {
    return;
}

static void sigusr2_cb(evutil_socket_t sig, short events, void* user_data) {
    return;
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

    printf("%s\r\n", query);

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

    int items = barn_query(context->db, query);

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

    int rc = create_save(file_name);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to create new save at %s\r\n", file_name);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    rc = open_save(file_name, &context->db);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to open new save at %s for inital settings\r\n", file_name);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    rc = add_barn_meta_property(context->db, "Level", 1);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to add inital settings\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    rc = add_silo_meta_property(context->db, "Level", 1);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to add inital settings\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    rc = add_barn_meta_property(context->db, "MaxCompacity", 50);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to add inital settings\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    rc = add_silo_meta_property(context->db, "MaxCompacity", 50);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to add inital settings\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    rc = add_meta_property(context->db, "Money", 1000);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to add inital settings\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    rc = add_meta_property(context->db, "Level", 1);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to add inital settings\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    rc = add_meta_property(context->db, "xp", 0);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to add inital settings\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    close_save(context->db);

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

    int rc = open_save(file_name, &context->db);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to open save at %s\r\n", file_name);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
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

    close_save(context->db);

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "save closed\r\n");
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
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
