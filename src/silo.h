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

#ifndef SILO_H
#define SILO_H

#include <event2/event.h>
#include <event2/buffer.h>

#include "save.h"
#include "list.h"

struct evbuffer* silo_query(sqlite3* db, const char* item, int* code);

struct evbuffer* silo_allocation(sqlite3* db, int* code);

struct evbuffer* silo_max(sqlite3* db, int* code);

struct evbuffer* silo_level(sqlite3* db, int* code);

struct evbuffer* upgrade_silo(sqlite3* db, int* code);

#endif /* SILO_H */
