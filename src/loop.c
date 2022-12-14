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

#define X(a, b, c) [a]={c, 0}
const struct timeval field_time[] = {
    FIELD_CROP_TABLE
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

    if (evhttp_set_cb(context->event_box->http_base, "/field/plant", plant_cb, context)) {
        syslog(LOG_WARNING, "failed to set /field/plant");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/field/harvest", fields_harvest_cb, context)) {
        syslog(LOG_WARNING, "failed to set /field/harvest");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/field/status", fields_cb, context)) {
        syslog(LOG_WARNING, "failed to set /field/status");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/field/buy", buy_field_cb, context)) {
        syslog(LOG_WARNING, "failed to set /field/buy");
        abort();
    }

    if (evhttp_set_cb(context->event_box->http_base, "/buy/field", buy_field_cb, context)) {
        syslog(LOG_WARNING, "failed to set /buy/field");
        abort();
    }


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

    rc = add_inital_save_values(context->db);
    if (rc != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "failed to add inital settings\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    close_save(context->db);
    context->db = NULL;

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

    //make fields list here
    context->field_list = make_fields_list(get_number_of_fields(context->db));

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
    context->db = NULL;

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
        }
        else {
            complete = "not ready";
        }
        evbuffer_add_printf(returnbuffer, "field%d: %s %s\r\n", list->field_number, field_crop_enum_to_string(list->type), complete);
    } while (list->next != NULL);

    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
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
        if (list->completion == 1) {
            if (get_silo_allocation(context->db) < get_silo_max(context->db)) {
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
                list->type = NONE_FIELD;
                list->completion = 0;
                update_meta(context->db, 2, "xp");
                xp_check(context->db);
            }
            else {
                struct evbuffer* returnbuffer = evbuffer_new();
                evbuffer_add_printf(returnbuffer, "could not harvest field%d due to silo size\r\n", list->field_number);
                evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                evbuffer_free(returnbuffer);
                return;
            }
            
        }
    } while (list->next != NULL);

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
    }

    if (get_number_of_fields_list(context->field_list) < get_number_of_fields(context->db)) {
        if (amend_fields_list(context->field_list, get_number_of_fields(context->db)) != 0) {
            struct evbuffer* returnbuffer = evbuffer_new();
            evbuffer_add_printf(returnbuffer, "could not amend fields\r\n");
            evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
            evbuffer_free(returnbuffer);
            return;
        }
    }

    const struct evhttp_uri* uri_struct = evhttp_request_get_evhttp_uri(req);

    const char* query = evhttp_uri_get_query(uri_struct);

    enum field_crop type;

    type = field_crop_string_to_enum(query);
    if (type == NONE_FIELD) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "%s is not a correct query\r\n", query);
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (get_skill_status(context->db, field_crop_enum_to_string(type))) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "currently do not own %s skill\r\n", field_crop_enum_to_string(type));
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    fields_list* list = context->field_list;

    int planted = 0;

    do {
        if (list->type == NONE_FIELD) {
            if (list->event == NULL) {
                list->event = event_new(context->event_box->base, -1, 0, field_ready_cb, list);
                if (list->event == NULL) {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "error making event\r\n");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    syslog(LOG_WARNING, "error making event for fields");
                    return;
                }
            }
            //consume crops or cash
            if (silo_query(context->db, query) > 0) {
                if (update_silo(context->db, query, -1) != 0) {
                    struct evbuffer* returnbuffer = evbuffer_new();
                    evbuffer_add_printf(returnbuffer, "could not update silo\r\n");
                    evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
                    evbuffer_free(returnbuffer);
                    return;
                }
            }
            else if (get_money(context->db) > 10) {
                if (update_meta(context->db, -10, "Money") != 0) {
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
    } while (list->next != NULL);


    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "planted: %d\r\n", planted);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
}

static void field_ready_cb(evutil_socket_t fd, short events, void* user_data) {
    fields_list* list = user_data;

    list->completion = 1;

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
        level_up(db, xp_needed);
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
            if (update_meta(context->db, (-1 * (pow(2, current))), "Money")) {
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

    if (amend_fields_list(context->field_list, get_number_of_fields(context->db)) != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "error amending field list\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        syslog(LOG_WARNING, "error amending field list");
        return;
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

    evhttp_send_reply(req, HTTP_NOTIMPLEMENTED, "Client", NULL);
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

    if (strcmp(query, "Fields")) {
        //nop
    }
    else if (strcmp(query, "TreePlots")) {
        //nop
    }
    else if (get_skill_status(context->db, query) != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "already own skill\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    int skill_points = get_skill_points(context->db);
    if (skill_points < 1) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "not enough skill points\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }
    if (update_meta(context->db, -1, "SkillPoints") != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "error subtracting skill points\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }
    if (update_skill_tree(context->db, query) != 0) {
        struct evbuffer* returnbuffer = evbuffer_new();
        evbuffer_add_printf(returnbuffer, "error adding skill\r\n");
        evhttp_send_reply(req, HTTP_INTERNAL, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    struct evbuffer* returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "sucessfully bought skill: %s\r\n", query);
    evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
    evbuffer_free(returnbuffer);
    return;
}
