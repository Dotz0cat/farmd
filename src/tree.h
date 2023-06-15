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

#ifndef TREE_H
#define TREE_H

#include <event2/event.h>
#include <event2/buffer.h>

#include "save.h"
#include "list.h"
#include "meta.h"
#include "storage.h"
#include "box_struct.h"

struct evbuffer *buy_tree_plot(sqlite3 *db, trees_list **tree_list, int *code);
struct evbuffer *plant_tree(sqlite3 *db, trees_list **tree_list, const char *crop, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *user_data), int *code);
struct evbuffer *harvest_tree(sqlite3 *db, trees_list *tree_list, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *user_data), int *code);
struct evbuffer *tree_status(sqlite3 *db, trees_list *tree_list, int *code);

void populate_trees(sqlite3 *db, trees_list **tree_list, struct event_base *base, void (*mature_cb)(evutil_socket_t fd, short events, void *arg), void (*ready_cb)(evutil_socket_t fd, short events, void *arg));
void free_trees(trees_list **list);
void ping_trees(sqlite3 *db);
void tree_mature_set(struct box_for_list_and_db *box, void (*cb)(evutil_socket_t fd, short events, void *arg));
void tree_complete_set(struct box_for_list_and_db *box);

#endif /* TREE_H */
