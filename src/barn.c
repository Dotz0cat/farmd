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

#include "barn.h"

struct evbuffer *barn_query(sqlite3 *db, const char *item, int *code) {
   struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    int number_of_items = barn_query_db(db, item);

    evbuffer_add_printf(returnbuffer, "%s: %i\r\n", item, number_of_items);

    SET_CODE_OK(code)

    return returnbuffer;
}

struct evbuffer *barn_allocation(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    int max = get_barn_max(db);

    int used = get_barn_allocation(db);

    float allocation = (float) used / (float) max;

    allocation = allocation * 100;

    evbuffer_add_printf(returnbuffer, "barn used: %.2f%%\r\n", allocation);

    SET_CODE_OK(code)

    return returnbuffer;
}

struct evbuffer *barn_max(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    int max = get_barn_max(db);

    evbuffer_add_printf(returnbuffer, "Barn max: %d\r\n", max);

    SET_CODE_OK(code)

    return returnbuffer;
}

struct evbuffer *barn_level(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    int barn_level = get_barn_meta_property(db, "Level");

    evbuffer_add_printf(returnbuffer, "Barn Level: %d\r\n", barn_level);

    SET_CODE_OK(code)

    return returnbuffer;
}

struct evbuffer *upgrade_barn(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    //get current
    int current_level = get_barn_meta_property(db, "Level");

    //check eligibility
    int current_farm_level = get_level(db);

    //1 barn level for every 3 farm levels
    if (current_level > (current_farm_level / 3)) {
        evbuffer_add_printf(returnbuffer, "insuffecent farm level to upgrade\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //calculate ammounts
    //cost scales with barn level
    //money and special item that I haven't came up with.
    int amount;
    int money_amount;
    //Yes this does use integer division. I want it to be lazy
    amount = 3 + (current_level / 2);
    money_amount = (500 * current_level);

    //name lookup
    const char *upgrade_item = special_item_enum_to_string(BARN_UPGRADE_ITEM);

    if (items_available(db, upgrade_item, amount) != 0) {
        evbuffer_add_printf(returnbuffer, "insuffecent items to upgrade\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //subtract items
    switch (subtract_money(db, money_amount)) {
        case (NO_MONEY_ERROR): {
            break;
        }
        case (ERROR_UPDATING): {
            evbuffer_add_printf(returnbuffer, "could not subtract money\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
        }
        case (NOT_ENOUGH): {
            evbuffer_add_printf(returnbuffer, "insuffecent money to upgrade\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
        }
    }

    switch (remove_from_storage(db, upgrade_item, amount)) {
        case (NO_STORAGE_ERROR): {
            break;
        }
        case (BARN_UPDATE): {
            evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
        case (BARN_SIZE): {
            evbuffer_add_printf(returnbuffer, "not enough %s in barn\r\n", upgrade_item);
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
        case (SILO_UPDATE): {
            evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
        case (SILO_SIZE): {
            evbuffer_add_printf(returnbuffer, "not enough %s in silo\r\n", upgrade_item);
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
        case (BARN_ADD):
        case (SILO_ADD):
        case (STORAGE_NOT_HANDLED): {
            evbuffer_add_printf(returnbuffer, "storage not handled\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
    }

    //upgrade
    if (update_barn_meta_property(db, "Level", 1) != 0) {
        evbuffer_add_printf(returnbuffer, "error updating level\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    int capacity = 0;

    if (current_level == 1) {
        capacity = 50;
    }
    else if (current_level > 1 && current_level < 10) {
        capacity = 100 << (current_level / 9);
    }
    else {
        capacity = 100 << (current_level / 10);
    }

    if (update_barn_meta_property(db, "MaxCapacity", capacity) != 0) {
        evbuffer_add_printf(returnbuffer, "error updating max capacity\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    SET_CODE_OK(code)
    evbuffer_add_printf(returnbuffer, "sucessfully upgrade barn\r\n");
    return returnbuffer;
}
