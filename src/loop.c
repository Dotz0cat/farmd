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

    //dbus stuff
    DBusError err;

    dbus_error_init(&err);

    DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

    if (conn == NULL || dbus_error_is_set(&err)) {
        syslog(LOG_WARNING, "could not connect to dbus");
        dbus_error_free(&err);
        return;
    }

    int ret = dbus_bus_request_name(conn, "Dotz0cat.Farmd", 0, &err);

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER || dbus_error_is_set(&err)) {
        syslog(LOG_WARNING, "could not become primary owner of bus.");
        dbus_error_free(&err);
        return;
    }

    context->conn = conn;

    context->event_box->dbus_dispatch = event_new(context->event_box->base, -1, EV_TIMEOUT, handle_dbus_dispatch_cb, (void*) context);
    if (context->event_box->dbus_dispatch == NULL) {
        abort();
    }

    ret = dbus_connection_set_watch_functions(conn, add_dbus_watch, remove_dbus_watch, toggle_dbus_watch, context, NULL);
    if (ret != 0) {
        abort();
    }

    ret = dbus_connection_set_timeout_functions(conn, add_dbus_timeout, remove_dbus_timeout, toggle_dbus_timeout, context, NULL);
    if (ret != TRUE) {
        abort();
    }

    ret = dbus_connection_add_filter(conn, dbus_message_filter, context, NULL);
    if (ret != TRUE) {
        abort();
    }

    dbus_connection_set_dispatch_status_function(conn, handle_dbus_dispatch_status, context, NULL);

    DBusObjectPathVTable* vtable = malloc(sizeof(DBusObjectPathVTable));
    if (vtable == NULL) {
        abort();
    }

    vtable->message_function = dbus_message_handler;
    vtable->unregister_function = dbus_unregister;

    ret = dbus_connection_register_object_path(conn, "/Dotz0cat/farmd", vtable, context);
    if (ret == FALSE) {
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

static dbus_bool_t add_dbus_watch(DBusWatch* watch, void* data) {
    if (!dbus_watch_get_enabled(watch)) {
        return FALSE;
    }
    loop_context* context = data;

    int fd = dbus_watch_get_unix_fd(watch);

    unsigned int flags = dbus_watch_get_flags(watch);
    short cond = EV_PERSIST;
    if (flags & DBUS_WATCH_READABLE) {
        cond |= EV_READ;
    }
    if (flags & DBUS_WATCH_WRITABLE){
        cond |= EV_WRITE;
    }

    watch_list w;
    w.watch = watch;
    w.context = context;

    struct event* event = event_new(context->event_box->base, fd, cond, dbus_watch_cb, &w);

    event_add(event, NULL);

    dbus_watch_set_data(watch, event, NULL);

    return TRUE;
}

static void remove_dbus_watch(DBusWatch* watch, void* data) {
    struct event* event = dbus_watch_get_data(watch);

    if (event != NULL) {
        event_free(event);
    }

    dbus_watch_set_data(watch, NULL, NULL);
}

static void toggle_dbus_watch(DBusWatch* watch, void* data) {
    if (dbus_watch_get_enabled(watch)) {
        add_dbus_watch(watch, data);
    }
    else {
        remove_dbus_watch(watch, data);
    }
}

static void dbus_watch_cb(int fd, short events, void* user_data) {
    watch_list* list = user_data;

    unsigned int flags = 0;
    if (events & EV_READ)
        flags |= DBUS_WATCH_READABLE;
    if (events & EV_WRITE)
        flags |= DBUS_WATCH_WRITABLE;

     if (dbus_watch_handle(list->watch, flags) == FALSE) {
        syslog(LOG_WARNING, "error with dbus_watch_handle");
     }

     handle_dbus_dispatch_status(list->context->conn, DBUS_DISPATCH_DATA_REMAINS, list->context);
}

//
static dbus_bool_t add_dbus_timeout(DBusTimeout* timeout, void* data) {
    if (!dbus_timeout_get_enabled(timeout)) {
        return FALSE;
    }
    loop_context* context = data;

    struct event* event = event_new(context->event_box->base, -1, EV_TIMEOUT|EV_PERSIST, dbus_timeout_cb, timeout);

    int ms = dbus_timeout_get_interval(timeout);
    struct timeval tv = {
        .tv_sec = ms / 1000,
        .tv_usec = (ms % 1000) * 1000,
    };
    event_add(event, &tv);

    dbus_timeout_set_data(timeout, event, NULL);

    return TRUE;
}

static void remove_dbus_timeout(DBusTimeout* timeout, void* data) {
    struct event* event = dbus_timeout_get_data(timeout);

    if (event != NULL) {
        event_free(event);
    }

    dbus_timeout_set_data(timeout, NULL, NULL);
}

static void toggle_dbus_timeout(DBusTimeout* timeout, void* data) {
    if (dbus_timeout_get_enabled(timeout)) {
        add_dbus_timeout(timeout, data);
    }
    else {
        remove_dbus_timeout(timeout, data);
    }
}

static void dbus_timeout_cb(int fd, short events, void* user_data) {
    dbus_timeout_handle( (DBusTimeout*) user_data);
}

static void handle_dbus_dispatch_status(DBusConnection* conn, DBusDispatchStatus status, void* data) {
    loop_context* context = data;

    if (status == DBUS_DISPATCH_DATA_REMAINS) {
        struct timeval tv = {
            .tv_sec = 0,
            .tv_usec = 0,
        };
        event_add(context->event_box->dbus_dispatch, &tv);
    }
}

static void handle_dbus_dispatch_cb(int fd, short events, void* user_data) {
    loop_context* context = user_data;

    DBusConnection* conn = context->conn;

    if (dbus_connection_get_dispatch_status(conn) == DBUS_DISPATCH_DATA_REMAINS) {
        dbus_connection_dispatch(conn);
    }
}

static DBusHandlerResult dbus_message_filter(DBusConnection* conn, DBusMessage* msg, void* data) {
    //just an introsector
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult dbus_message_handler(DBusConnection* conn, DBusMessage* msg, void* data) {
    //handle here
    loop_context* context = data;
    int type = dbus_message_get_type(msg);

    switch(type) {
        case DBUS_MESSAGE_TYPE_METHOD_CALL:
            dbus_message_get_interface(msg);
        break;
        case DBUS_MESSAGE_TYPE_METHOD_RETURN:
            //stuff
        break;
        case DBUS_MESSAGE_TYPE_ERROR:
            //stuff
        break;
        case DBUS_MESSAGE_TYPE_SIGNAL:
            //stuff
        break;
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

static void dbus_unregister(DBusConnection* conn, void* data) {
    return;
}

// static DBusHandlerResult dbus_meta_message_handler(DBusConnection* conn, DBusMessage* msg, void* data) {

// }
