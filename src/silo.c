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

#include "silo.h"

struct evbuffer *silo_query(sqlite3 *db, const char *item, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    int number_of_items = silo_query_db(db, item);

    evbuffer_add_printf(returnbuffer, "%s: %i\r\n", item, number_of_items);

    *code = 200;

    return returnbuffer;
}

struct evbuffer *silo_allocation(sqlite3 *db, int *code) {
    int max = get_silo_max(db);

    int used = get_silo_allocation(db);

    float allocation = (float) used / (float) max;

    allocation = allocation * 100;

    struct evbuffer *returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "silo used: %.2f%%\r\n", allocation);

    *code = 200;

    return returnbuffer;
}

struct evbuffer *silo_max(sqlite3 *db, int *code) {
    int max = get_silo_max(db);

    struct evbuffer *returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "Silo max: %d\r\n", max);

    *code = 200;

    return returnbuffer;
}

struct evbuffer *silo_level(sqlite3 *db, int *code) {
    int barn_level = get_silo_meta_property(db, "Level");

    struct evbuffer *returnbuffer = evbuffer_new();
    evbuffer_add_printf(returnbuffer, "Silo Level: %d\r\n", barn_level);

    *code = 200;

    return returnbuffer;
}

struct evbuffer *upgrade_silo(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();
    //get current
    int current_level = get_silo_meta_property(db, "Level");

    //check eligibility
    int current_farm_level = get_level(db);

    //1 silo level for every 3 farm levels
    if (current_level > (current_farm_level / 3)) {
        evbuffer_add_printf(returnbuffer, "insuffecent farm level to upgrade\r\n");
        *code = 500;
        return returnbuffer;
    }

    //calculate ammounts
    //cost scales with silo level
    //money and special item that I haven't came up with.
    int amount;
    int money_amount;
    //Yes this does use integer division. I want it to be lazy
    amount = 3 + (current_level / 2);
    money_amount = (500 * current_level);

    //name lookup
    const char *upgrade_item = special_item_enum_to_string(SILO_UPGRADE_ITEM);
    //type lookup even though it will be barn
    enum storage store = get_storage_type_special(SILO_UPGRADE_ITEM);

    if (store == BARN) {
        //check items
        if (barn_query_db(db, upgrade_item) < amount || get_money(db) < money_amount) {
            evbuffer_add_printf(returnbuffer, "insuffecent items to upgrade\r\n");
            *code = 500;
            return returnbuffer;
        }
    }
    else if (store == SILO) {
        //check items
        if (silo_query_db(db, upgrade_item) < amount || get_money(db) < money_amount) {
            evbuffer_add_printf(returnbuffer, "insuffecent items to upgrade\r\n");
            *code = 500;
            return returnbuffer;
        }
    }
    else {
        evbuffer_add_printf(returnbuffer, "This error is not supposed to happen\r\n%s not placed in barn or silo\r\n", upgrade_item);
        *code = 500;
        return returnbuffer;
    }

    //subtract items
    if (update_meta(db, (-1 * money_amount), "Money") != 0) {
        evbuffer_add_printf(returnbuffer, "could not subtract money\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (store == BARN) {
        if (update_barn(db, upgrade_item, (-1 * amount)) != 0) {
            evbuffer_add_printf(returnbuffer, "could not subtract item\r\n");
            *code = 500;
            return returnbuffer;
        }
    }
    else if (store == SILO) {
        if (update_silo(db, upgrade_item, (-1 * amount)) != 0) {
            evbuffer_add_printf(returnbuffer, "could not subtract item\r\n");
            *code = 500;
            return returnbuffer;
        }
    }
    else {
        evbuffer_add_printf(returnbuffer, "This error is not supposed to happen\r\n%s not placed in barn or silo\r\n", upgrade_item);
        *code = 500;
        return returnbuffer;
    }

    //upgrade
    if (update_silo_meta_property(db, "Level", 1) != 0) {
        evbuffer_add_printf(returnbuffer, "error updating level\r\n");
        *code = 500;
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

    if (update_silo_meta_property(db, "MaxCapacity", capacity) != 0) {
        evbuffer_add_printf(returnbuffer, "error updating max capacity\r\n");
        *code = 500;
        return returnbuffer;
    }

    *code = 200;
    evbuffer_add_printf(returnbuffer, "sucessfully upgrade silo\r\n");
    return returnbuffer;
}
