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
