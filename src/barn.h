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

#ifndef BARN_H
#define BARN_H

#include <event2/event.h>
#include <event2/buffer.h>

#include "save.h"
#include "list.h"
#include "storage.h"
#include "meta.h"

struct evbuffer *barn_query(sqlite3 *db, const char *item, int *code);
struct evbuffer *barn_allocation(sqlite3 *db, int *code);
struct evbuffer *barn_max(sqlite3 *db, int *code);
struct evbuffer *barn_level(sqlite3 *db, int *code);
struct evbuffer *upgrade_barn(sqlite3 *db, int *code);
struct evbuffer *barn_upgrade_cost(sqlite3 *db, int *code);
struct evbuffer *barn_next_level_stats(sqlite3 *db, int *code);

#endif /* BARN_H */
