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

#ifndef META_H
#define META_H

#include <event2/buffer.h>

#include "save.h"
#include "list.h"
#include "storage.h"

#define CHECK_SAVE_OPEN(a, b, c) \
if (a == NULL) { \
    evbuffer_add_printf(b, "no save open\r\n"); \
    *c = 500; \
    return b; \
}

#define SET_CODE_INTERNAL_ERROR(a) \
*a = 500;

#define SET_CODE_OK(a) \
*a = 200;

enum money_errors {
    NO_MONEY_ERROR,
    NOT_ENOUGH,
    ERROR_UPDATING,
};

enum consume_or_buy_errors {
    NO_CONSUME_OR_BUY_ERROR,
    CONSUME_OR_BUY_BARN,
    CONSUME_OR_BUY_SILO,
    CONSUME_OR_BUY_NOT_ENOUGH_MONEY,
    CONSUME_OR_BUY_MONEY_ERROR,
    COULD_NOT_CONSUME_OR_BUY,
};

void xp_check(sqlite3 *db);
struct evbuffer *view_money(sqlite3 *db, int *code);
struct evbuffer *view_level(sqlite3 *db, int *code);
struct evbuffer *view_xp(sqlite3 *db, int *code);
struct evbuffer *view_skill_points(sqlite3 *db, int *code);

enum money_errors add_money(sqlite3 *db, const int amount);
enum money_errors subtract_money(sqlite3 *db, const int amount);

enum consume_or_buy_errors consume_crops_or_cash(sqlite3 *db, const char *item, const int price);

int money_check(sqlite3 *db, const int amount);

#endif /* META_H */
