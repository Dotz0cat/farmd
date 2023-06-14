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

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    enum item_type type = get_product_type_string(item);

    int item_price = 0;
    const char *sanitized_string;

    switch (type) {
        case NONE_PRODUCT:
            item_price = 0;
            break;
        case FIELD_PRODUCT: {
            enum field_crop item_enum = field_crop_string_to_enum(item);
            item_price = field_crop_buy_cost(item_enum);
            sanitized_string = field_crop_enum_to_string(item_enum);
            break;
        }
        case TREE_PRODUCT: {
            enum tree_crop item_enum = tree_crop_string_to_enum(item);
            item_price = tree_crop_buy_cost(item_enum);
            sanitized_string = tree_crop_enum_to_string(item_enum);
            break;
        }
        case SPECIAL_PRODUCT: {
            enum special_item item_enum = special_item_string_to_enum(item);
            item_price = special_item_buy_cost(item_enum);
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
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //db work
    //update money
    switch (subtract_money(db, item_price)) {
        case (NO_MONEY_ERROR): {
            break;
        }
        case (NOT_ENOUGH): {
            evbuffer_add_printf(returnbuffer, "not enough money to buy item\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
        case (ERROR_UPDATING): {
            evbuffer_add_printf(returnbuffer, "error updating money\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
    }

    switch (add_to_storage(db, sanitized_string, 1)) {
        case (NO_STORAGE_ERROR): {
            break;
        }
        case (BARN_UPDATE): {
            evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
        case (BARN_ADD): {
            evbuffer_add_printf(returnbuffer, "error adding item to barn\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
        case (BARN_SIZE): {
            evbuffer_add_printf(returnbuffer, "error buying due to barn size\r\n");
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
        case (SILO_ADD): {
            evbuffer_add_printf(returnbuffer, "error adding item to silo\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
        case (SILO_SIZE): {
            evbuffer_add_printf(returnbuffer, "error buying due to silo size\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
        case (STORAGE_NOT_HANDLED): {
            evbuffer_add_printf(returnbuffer, "storage not handled\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
    }

    evbuffer_add_printf(returnbuffer, "sucessfully bought: %s\r\n", sanitized_string);

    SET_CODE_OK(code)
    return returnbuffer;
}

struct evbuffer *sell_item(sqlite3* db, const char *item, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    enum item_type type =  get_product_type_string(item);

    int item_price = 0;
    const char *sanitized_string;

    switch (type) {
        case NONE_PRODUCT:
            item_price = 0;
            break;
        case FIELD_PRODUCT: {
            enum field_crop item_enum = field_crop_string_to_enum(item);
            item_price = field_crop_sell_cost(item_enum);
            sanitized_string = field_crop_enum_to_string(item_enum);
            break;
        }
        case TREE_PRODUCT: {
            enum tree_crop item_enum = tree_crop_string_to_enum(item);
            item_price = tree_crop_sell_cost(item_enum);
            sanitized_string = tree_crop_enum_to_string(item_enum);
            break;
        }
        case SPECIAL_PRODUCT: {
            enum special_item item_enum = special_item_string_to_enum(item);
            item_price = special_item_sell_cost(item_enum);
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
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    switch (remove_from_storage(db, sanitized_string, 1)) {
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
            evbuffer_add_printf(returnbuffer, "not enough %s in barn\r\n", sanitized_string);
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
            evbuffer_add_printf(returnbuffer, "not enough %s in silo\r\n", sanitized_string);
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

    //update money
    switch (add_money(db, item_price)) {
        case (NO_MONEY_ERROR): {
            break;
        }
        case (NOT_ENOUGH):
        case (ERROR_UPDATING): {
            evbuffer_add_printf(returnbuffer, "error updating money\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
            break;
        }
    }

    evbuffer_add_printf(returnbuffer, "sucessfully sold: %s for %d\r\n", sanitized_string, item_price);

    SET_CODE_OK(code)
    return returnbuffer;
}

struct evbuffer *get_item_buy_price(sqlite3 *db, const char *item_name, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

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

    SET_CODE_OK(code)
    return returnbuffer;
}

struct evbuffer *get_item_sell_price(sqlite3 *db, const char *item_name, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

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

    SET_CODE_OK(code)
    return returnbuffer;
}
