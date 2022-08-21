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

    if (open_save("/home/seth/Files/farmd_test", context->db) != 0) {
        syslog(LOG_WARNING, "could not open %s", "filename");
        abort();
    }

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

    context->event_box->rpc_base = evrpc_init(context->event_box->http_base);

    EVRPC_REGISTER(context->event_box->rpc_base, BarnQuery, BarnQueryRequest, BarnQueryReply, barn_query_cb, context);
    EVRPC_REGISTER(context->event_box->rpc_base, SiloQuery, SiloQueryRequest, SiloQueryReply, silo_query_cb, context);

    evhttp_set_gencb(context->event_box->http_base, generic_http_cb, context);

    evrpc_add_hook(context->event_box->rpc_base, INPUT, rpc_hook_cb, context);

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
    struct evbuffer *returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "nothing available at %s\r\n", evhttp_request_get_uri(req));
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void barn_query_cb(EVRPC_STRUCT(BarnQuery)* rpc, void* arg) {
    struct BarnQueryRequest* request = rpc->request;
    struct BarnQueryReply* reply = rpc->reply;

    loop_context* context = arg;

    if (EVTAG_HAS(request, item_name)) {
        char* name_of_item;
        EVTAG_GET(request, item_name, &name_of_item);

        int item_amount = barn_query(context->db, name_of_item);

        EVTAG_ASSIGN(reply, items, item_amount);
    }
    else {
        EVTAG_ASSIGN(reply, items, -1);
    }

    EVRPC_REQUEST_DONE(rpc);
}

static void silo_query_cb(EVRPC_STRUCT(SiloQuery)* rpc, void* arg) {
    struct SiloQueryRequest* request = rpc->request;
    struct SiloQueryReply* reply = rpc->reply;

    loop_context* context = arg;

    char* name_of_item;
    EVTAG_GET(request, item_name, &name_of_item);

    int item_amount = silo_query(context->db, name_of_item);

    EVTAG_ASSIGN(reply, items, 20);

    EVRPC_REQUEST_DONE(rpc);
}

// static void barn_query_uri_cb(struct evhttp_request* req, void* arg) {
//     EVRPC_MAKE_REQUEST(BarnQuery, pool, BarnQueryRequest, BarnQueryReply, cb, cbarg)
// }

static void logging_cb(int severity, const char *msg) {
    syslog(LOG_NOTICE, "event %i: %s", severity, msg);
}

static int rpc_hook_cb(void* thing, struct evhttp_request* request, struct evbuffer* buff, void* data) {
    return EVRPC_CONTINUE;
}