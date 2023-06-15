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

#ifndef FIELD_PRIVATE_H
#define FIELD_PRIVATE_H

#include "field.h"

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]={c, 0}
static const struct timeval field_time[] = {
    FIELD_CROP_TABLE
};
#undef X

static void setup_field_completion(sqlite3 *db, fields_list *list, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg));

#endif /* FIELD_PRIVATE_H */
