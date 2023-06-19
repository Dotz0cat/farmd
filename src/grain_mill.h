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

#ifndef GRAIN_MILL_H
#define GRAIN_MILL_H

#include <event2/event.h>
#include <event2/buffer.h>

#include "save.h"
#include "list.h"
#include "storage.h"
#include "meta.h"
#include "box_struct.h"

void populate_grain_mill(sqlite3 *db, queue_list **queue, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg));
void ping_grain_mill(sqlite3 *db);
void free_queue_list(queue_list **head);
void free_slot_list(slot_list **head);
struct evbuffer *buy_grain_mill(sqlite3 *db, queue_list **queue, int *code);
struct evbuffer *get_grain_mill_buy_cost(sqlite3 *db, int *code);
struct evbuffer *upgrade_grain_mill(sqlite3 *db, queue_list **queue, int *code);
struct evbuffer *get_grain_mill_upgrade_cost(sqlite3 *db, int *code);
struct evbuffer *get_grain_mill_next_level_stats(sqlite3 *db, int *code);
struct evbuffer *collect_grain_mill_procducts(sqlite3 *db, queue_list *queue, int *code);
struct evbuffer *add_item_to_grain_mill_queue(sqlite3 *db, queue_list *queue, const char *product, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg), int *code);
void mark_grain_mill_item_as_complete(sqlite3 *db, queue_list *queue);

#endif /* GRAIN_MILL_H */
