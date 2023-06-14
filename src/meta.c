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

#include "meta.h"

void xp_check(sqlite3 *db) {
    int level = get_level(db);
    level--;
    //was pow(2, level) * 10;
    int xp_needed = 10 << level;
    //xp needed for level 2 is 10
    //level 3 is 20
    //level 4 is 40
    //level 5 is 80
    //level 6 is 160
    int xp = get_xp(db);
    if (xp >= xp_needed) {
        if (level_up(db, xp_needed) != 0) {
            syslog(LOG_WARNING, "error leveling up");
            return;
        }
        if (update_meta(db, 2, "SkillPoints") != 0) {
            syslog(LOG_WARNING, "error adding skill points");
            return;
        }
    }
}

struct evbuffer *view_money(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    int money = get_money(db);
    evbuffer_add_printf(returnbuffer, "money: %d\r\n", money);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *view_level(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    int level = get_level(db);
    evbuffer_add_printf(returnbuffer, "level: %d\r\n", level);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *view_xp(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    int xp = get_xp(db);
    evbuffer_add_printf(returnbuffer, "xp: %d\r\n", xp);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *view_skill_points(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    int skill_points = get_skill_points(db);
    evbuffer_add_printf(returnbuffer, "Skill points: %d\r\n", skill_points);

    *code = 200;
    return returnbuffer;
}

enum money_errors add_money(sqlite3 *db, const int amount) {
    if (update_meta(db, amount, "Money") != 0) {
        return ERROR_UPDATING;
    }

    return NO_MONEY_ERROR;
}

enum money_errors subtract_money(sqlite3 *db, const int amount) {
    if (get_money(db) > amount) {
        if (update_meta(db, (-1 * amount), "Money") != 0) {
            return ERROR_UPDATING;
        }
    }
    else {
        return NOT_ENOUGH;
    }

    return NO_MONEY_ERROR;
}

enum consume_or_buy_errors consume_crops_or_cash(sqlite3 *db, const char *item) {
    switch (remove_from_storage(db, item, 1)) {
        case (NO_STORAGE_ERROR): {
            return NO_CONSUME_OR_BUY_ERROR;
            break;
        }
        case (BARN_UPDATE): {
            return CONSUME_OR_BUY_BARN;
            break;
        }
        case (SILO_UPDATE): {
            return CONSUME_OR_BUY_SILO;
            break;
        }
        case (BARN_SIZE):
        case (SILO_SIZE): {
            int price = item_buy_price_string(item);
            switch (subtract_money(db, price)) {
                case (NO_MONEY_ERROR): {
                    return NO_CONSUME_OR_BUY_ERROR;
                    break;
                }
                case (NOT_ENOUGH): {
                    return CONSUME_OR_BUY_NOT_ENOUGH_MONEY;
                    break;
                }
                case (ERROR_UPDATING): {
                    return CONSUME_OR_BUY_MONEY_ERROR;
                }
            }
            break;
        }
        case (BARN_ADD):
        case (SILO_ADD):
        case (STORAGE_NOT_HANDLED):
        default: {
            return COULD_NOT_CONSUME_OR_BUY;
            break;
        }
    }
    return COULD_NOT_CONSUME_OR_BUY;
}

enum consume_or_buy_errors consume_crops_or_cash_price_hint(sqlite3 *db, const char *item, const int price) {
    switch (remove_from_storage(db, item, 1)) {
        case (NO_STORAGE_ERROR): {
            return NO_CONSUME_OR_BUY_ERROR;
            break;
        }
        case (BARN_UPDATE): {
            return CONSUME_OR_BUY_BARN;
            break;
        }
        case (SILO_UPDATE): {
            return CONSUME_OR_BUY_SILO;
            break;
        }
        case (BARN_SIZE):
        case (SILO_SIZE): {
            switch (subtract_money(db, price)) {
                case (NO_MONEY_ERROR): {
                    return NO_CONSUME_OR_BUY_ERROR;
                    break;
                }
                case (NOT_ENOUGH): {
                    return CONSUME_OR_BUY_NOT_ENOUGH_MONEY;
                    break;
                }
                case (ERROR_UPDATING): {
                    return CONSUME_OR_BUY_MONEY_ERROR;
                }
            }
            break;
        }
        case (BARN_ADD):
        case (SILO_ADD):
        case (STORAGE_NOT_HANDLED):
        default: {
            return COULD_NOT_CONSUME_OR_BUY;
            break;
        }
    }
    return COULD_NOT_CONSUME_OR_BUY;
}
