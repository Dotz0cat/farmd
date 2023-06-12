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

#include "econ.h"

struct evbuffer *buy_item(sqlite3 *db, const char *item, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    enum item_type type =  get_product_type_string(item);

    int item_price = 0;
    enum storage storage_place;
    const char *sanitized_string;

    switch (type) {
        case NONE_PRODUCT:
            item_price = 0;
            break;
        case FIELD_PRODUCT: {
            enum field_crop item_enum = field_crop_string_to_enum(item);
            item_price = field_crop_buy_cost(item_enum);
            storage_place = get_storage_type_field(item_enum);
            sanitized_string = field_crop_enum_to_string(item_enum);
            break;
        }
        case TREE_PRODUCT: {
            enum tree_crop item_enum = tree_crop_string_to_enum(item);
            item_price = tree_crop_buy_cost(item_enum);
            storage_place = get_storage_type_tree(item_enum);
            sanitized_string = tree_crop_enum_to_string(item_enum);
            break;
        }
        case SPECIAL_PRODUCT: {
            enum special_item item_enum = special_item_string_to_enum(item);
            item_price = special_item_buy_cost(item_enum);
            storage_place = get_storage_type_special(item_enum);
            sanitized_string = special_item_enum_to_string(item_enum);
            break;
        }
        case OTHER_PRODUCT:
            //nop
            item_price = 0;
            break;
        default:
            item_price = 0;
            break;
    }

    if (item_price == 0) {
        evbuffer_add_printf(returnbuffer, "%s: is not valid\r\n", item);
        *code = 500;
        return returnbuffer;
    }

    //db work
    //update money
    if (get_money(db) > item_price) {
        if (update_meta(db, (-1 * item_price), "Money")) {
            evbuffer_add_printf(returnbuffer, "error updating money\r\n");
            *code = 500;
            return returnbuffer;
        }
    }
    else {
        evbuffer_add_printf(returnbuffer, "not enough money to buy item\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (storage_place == BARN) {
        //check allocation
        if (get_barn_allocation(db) < get_barn_max(db)) {
            //check if item is there
            if (barn_query_db(db, sanitized_string) != -1) {
                //add item
                if (update_barn(db, sanitized_string, 1) != 0) {
                    evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                    *code = 500;
                    return returnbuffer;
                }
            }
            else {
                if (type != SPECIAL_PRODUCT) {
                    if (get_skill_status(db, sanitized_string) == 0) {
                        if (add_item_to_barn(db, sanitized_string, LOCKED) != 0) {
                            evbuffer_add_printf(returnbuffer, "error adding item to barn\r\n");
                            *code = 500;
                            return returnbuffer;
                        }
                        if (update_barn(db, sanitized_string, 1) != 0) {
                            evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                            *code = 500;
                            return returnbuffer;
                        }
                    }
                    else {
                        if (add_item_to_barn(db, sanitized_string, UNLOCKED) != 0) {
                            evbuffer_add_printf(returnbuffer, "error adding item to barn\r\n");
                            *code = 500;
                            return returnbuffer;
                        }
                        if (update_barn(db, sanitized_string, 1) != 0) {
                            evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                            *code = 500;
                            return returnbuffer;
                        }
                    }
                }
                else {
                    //if it is a special product it is special
                    if (add_item_to_barn(db, sanitized_string, SPECIAL) != 0) {
                        evbuffer_add_printf(returnbuffer, "error adding item to barn\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                    if (update_barn(db, sanitized_string, 1) != 0) {
                        evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                }
            }
        }
        else {
            evbuffer_add_printf(returnbuffer, "error buying due to barn size\r\n");
            *code = 500;
            return returnbuffer;
        }
    }
    else if (storage_place == SILO) {
        //check allocation
        if (get_silo_allocation(db) < get_silo_max(db)) {
            //check if item is there
            if (silo_query_db(db, sanitized_string) != -1) {
                //add item
                if (update_silo(db, sanitized_string, 1) != 0) {
                    evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
                    *code = 500;
                    return returnbuffer;
                }
            }
            else {
                if (type != SPECIAL_PRODUCT) {
                    if (get_skill_status(db, sanitized_string) == 0) {
                        if (add_item_to_silo(db, sanitized_string, LOCKED) != 0) {
                            evbuffer_add_printf(returnbuffer, "error adding item to silo\r\n");
                            *code = 500;
                            return returnbuffer;
                        }
                        if (update_silo(db, sanitized_string, 1) != 0) {
                            evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
                            *code = 500;
                            return returnbuffer;
                        }
                    }
                    else {
                        if (add_item_to_silo(db, sanitized_string, UNLOCKED) != 0) {
                            evbuffer_add_printf(returnbuffer, "error adding item to silo\r\n");
                            *code = 500;
                            return returnbuffer;
                        }
                        if (update_silo(db, sanitized_string, 1) != 0) {
                            evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
                            *code = 500;
                            return returnbuffer;
                        }
                    }
                }
                else {
                    //if it is a special product it is special
                    if (add_item_to_barn(db, sanitized_string, SPECIAL) != 0) {
                        evbuffer_add_printf(returnbuffer, "error adding item to barn\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                    if (update_barn(db, sanitized_string, 1) != 0) {
                        evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                }
            }
        }
        else {
            evbuffer_add_printf(returnbuffer, "error buying due to silo size\r\n");
            *code = 500;
            return returnbuffer;
        }
    }
    else {
        *code = 500;
        return returnbuffer;
    }

    evbuffer_add_printf(returnbuffer, "sucessfully bought: %s\r\n", sanitized_string);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *sell_item(sqlite3* db, const char *item, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    enum item_type type =  get_product_type_string(item);

    int item_price = 0;
    enum storage storage_place;
    const char *sanitized_string;

    switch (type) {
        case NONE_PRODUCT:
            item_price = 0;
            break;
        case FIELD_PRODUCT: {
            enum field_crop item_enum = field_crop_string_to_enum(item);
            item_price = field_crop_sell_cost(item_enum);
            storage_place = get_storage_type_field(item_enum);
            sanitized_string = field_crop_enum_to_string(item_enum);
            break;
        }
        case TREE_PRODUCT: {
            enum tree_crop item_enum = tree_crop_string_to_enum(item);
            item_price = tree_crop_sell_cost(item_enum);
            storage_place = get_storage_type_tree(item_enum);
            sanitized_string = tree_crop_enum_to_string(item_enum);
            break;
        }
        case SPECIAL_PRODUCT: {
            enum special_item item_enum = special_item_string_to_enum(item);
            item_price = special_item_sell_cost(item_enum);
            storage_place = get_storage_type_special(item_enum);
            sanitized_string = special_item_enum_to_string(item_enum);
            break;
        }
        case OTHER_PRODUCT:
            //nop
            item_price = 0;
            break;
        default:
            item_price = 0;
            break;
    }

    if (item_price == 0) {
        evbuffer_add_printf(returnbuffer, "%s: is not valid\r\n", item);
        *code = 500;
        return returnbuffer;
    }

    //db work
    if (storage_place == BARN) {
        //check if item is there
        if (barn_query_db(db, sanitized_string) > 0) {
            //remove item
            if (update_barn(db, sanitized_string, -1) != 0) {
                evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
        else {
            evbuffer_add_printf(returnbuffer, "not enough %s in barn\r\n", sanitized_string);
            *code = 500;
            return returnbuffer;
        }
    }
    else if (storage_place == SILO) {
        //check if item is there
        if (silo_query_db(db, sanitized_string) > 0) {
            //remove item
            if (update_silo(db, sanitized_string, -1) != 0) {
                evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
        else {
            evbuffer_add_printf(returnbuffer, "not enough %s in silo\r\n", sanitized_string);
            *code = 500;
            return returnbuffer;
        }
    }
    else {
        *code = 500;
        return returnbuffer;
    }

    //update money
    if (update_meta(db, item_price, "Money")) {
        evbuffer_add_printf(returnbuffer, "error updating money\r\n");
        *code = 500;
        return returnbuffer;
    }

    evbuffer_add_printf(returnbuffer, "sucessfully sold: %s for %d\r\n", sanitized_string, item_price);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *get_item_buy_price(sqlite3 *db, const char *item_name, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    enum item_type type =  get_product_type_string(item_name);

    switch (type) {
        case NONE_PRODUCT:
            evbuffer_add_printf(returnbuffer, "%s: is not a valid item\r\n", item_name);
            break;
        case FIELD_PRODUCT: {
            enum field_crop item = field_crop_string_to_enum(item_name);
            evbuffer_add_printf(returnbuffer, "%s: %d\r\n", item_name, field_crop_buy_cost(item));
            break;
        }
        case TREE_PRODUCT: {
            enum tree_crop item = tree_crop_string_to_enum(item_name);
            evbuffer_add_printf(returnbuffer, "%s: %d\r\n", item_name, tree_crop_buy_cost(item));
            break;
        }
        case SPECIAL_PRODUCT: {
            enum special_item item = special_item_string_to_enum(item_name);
            evbuffer_add_printf(returnbuffer, "%s: %d\r\n", item_name, special_item_buy_cost(item));
            break;
        }
        case OTHER_PRODUCT:
            //nop
            evbuffer_add_printf(returnbuffer, "%s: not yet implemented\r\n", item_name);
            break;
        default:
            evbuffer_add_printf(returnbuffer, "%s: is not a valid item\r\n", item_name);
            break;
    }

    *code = 200;
    return returnbuffer;
}

struct evbuffer *get_item_sell_price(sqlite3 *db, const char *item_name, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    enum item_type type =  get_product_type_string(item_name);

    switch (type) {
        case NONE_PRODUCT:
            evbuffer_add_printf(returnbuffer, "%s: is not a valid item\r\n", item_name);
            break;
        case FIELD_PRODUCT: {
            enum field_crop item = field_crop_string_to_enum(item_name);
            evbuffer_add_printf(returnbuffer, "%s: %d\r\n", item_name, field_crop_sell_cost(item));
            break;
        }
        case TREE_PRODUCT: {
            enum tree_crop item = tree_crop_string_to_enum(item_name);
            evbuffer_add_printf(returnbuffer, "%s: %d\r\n", item_name, tree_crop_sell_cost(item));
            break;
        }
        case SPECIAL_PRODUCT: {
            enum special_item item = special_item_string_to_enum(item_name);
            evbuffer_add_printf(returnbuffer, "%s: %d\r\n", item_name, special_item_sell_cost(item));
            break;
        }
        case OTHER_PRODUCT:
            //nop
            evbuffer_add_printf(returnbuffer, "%s: not yet implemented\r\n", item_name);
            break;
        default:
            evbuffer_add_printf(returnbuffer, "%s: is not a valid item\r\n", item_name);
            break;
    }

    *code = 200;
    return returnbuffer;
}
