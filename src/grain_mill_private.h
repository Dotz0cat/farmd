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

#ifndef GRAIN_MILL_PRIVATE_H
#define GRAIN_MILL_PRIVATE_H

#include "grain_mill.h"

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]={c, 0}
static const struct timeval grain_mill_times[] = {
    PROCESSED_ITEM_TABLE
};
#undef X

#define GRAIN_MILL_BUY_COST 1000
#define GRAIN_MILL_LEVEL_COST 5

//product to make, single ingredient, ingredient type, type for union quanity for 1
#define GRAIN_MILL_RECIPE_TABLE \
X(WHEAT_FLOUR, WHEAT, FIELD_PRODUCT, field_item, 2), \
X(CORN_MEAL, CORN, FIELD_PRODUCT, field_item, 2),

#define X(a, b, c, d, e) [a]={a, {c, .d = b}, e}
static const struct recipes grain_mill_recipes[] = {
    GRAIN_MILL_RECIPE_TABLE
};
#undef X

struct time_and_ptr {
    time_t time;
    slot_list *slot;
};

static void setup_grain_mill_queue(sqlite3 *db, slot_list **slots, queue_list *queue, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg));
static struct time_and_ptr setup_grain_mill_completion(sqlite3 *db, slot_list *list, queue_list *queue, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg));

#endif /* GRAIN_MILL_PRIVATE_H */