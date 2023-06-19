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

#ifndef TYPE_H
#define TYPE_H

#include <string.h>

enum storage {
    NONE_STORAGE,
    BARN,
    SILO
};

enum item_type {
    NONE_PRODUCT,
    FIELD_PRODUCT,
    TREE_PRODUCT,
    GRAIN_MILL_PRODUCT,
    SUGAR_MILL_PRODUCT,
    SPECIAL_PRODUCT,
    OTHER_PRODUCT
};

//enum, string, time, buy, sell, storage, item_type
#define FIELD_CROP_TABLE \
X(NONE_FIELD, "none", 0, 0, 0, NONE_STORAGE, NONE_PRODUCT), \
X(WHEAT, "wheat", 30, 10, 8, SILO, FIELD_PRODUCT), \
X(CORN, "corn", 60, 10, 8, SILO, FIELD_PRODUCT), \
X(POTATOS, "potatos", 60, 10, 8, SILO, FIELD_PRODUCT), \
X(BEETS, "beets", 60, 10, 8, SILO, FIELD_PRODUCT), \
X(SUGARCANE, "sugarcane", 60, 10, 8, SILO, FIELD_PRODUCT), \
X(TURNIPS, "turnips", 60, 10, 8, SILO, FIELD_PRODUCT), \
X(TOMATOS, "tomatos", 60, 10, 8, SILO, FIELD_PRODUCT), \
X(CUCUMBERS, "cucumbers", 60, 10, 8, SILO, FIELD_PRODUCT), \
X(OKRA, "okra", 60, 10, 8, SILO, FIELD_PRODUCT)

#define X(a, b, c, d, e, f, g) a
enum field_crop {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type, maturity time
#define TREE_CROP_TABLE \
X(NONE_TREE, "none", 0, 0, 0, NONE_STORAGE, NONE_PRODUCT, 0), \
X(PEARS, "pears", 60, 10, 8, BARN, TREE_PRODUCT, 60), \
X(APPLES, "apples", 60, 10, 8, BARN, TREE_PRODUCT, 60), \
X(ORANGES, "oranges", 60, 10, 8, BARN, TREE_PRODUCT, 60), \
X(PEACHES, "peaches", 60, 10, 8, BARN, TREE_PRODUCT, 60)

#define X(a, b, c, d, e, f, g, h) a
enum tree_crop {
    TREE_CROP_TABLE
};
#undef X

//enum, string, buy, sell, item_type, storage
#define SPECIAL_ITEM_TABLE \
X(NONE_SPECIAL, "none", 0, 0, NONE_PRODUCT, NONE_STORAGE), \
X(BARN_UPGRADE_ITEM, "BarnUpgradeItem", 100, 2, SPECIAL_PRODUCT, BARN), \
X(SILO_UPGRADE_ITEM, "SiloUpgradeItem", 100, 2, SPECIAL_PRODUCT, BARN), \
X(GRAIN_MILL_UPGRADE_ITEM, "GrainMillUpgradeItem", 100, 2, SPECIAL_PRODUCT, BARN)

#define X(a, b, c, d, e, f) a
enum special_item {
    SPECIAL_ITEM_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define PROCESSED_ITEM_TABLE \
X(NONE_PROCESS, "none", 0, 0, 0, NONE_STORAGE, NONE_PRODUCT), \
X(WHEAT_FLOUR, "wheatFlour", 60, 20, 18, BARN, GRAIN_MILL_PRODUCT), \
X(CORN_MEAL, "cornMeal", 60, 20, 18, BARN, GRAIN_MILL_PRODUCT), \
X(BEET_SUGAR, "beetSugar", 60, 20, 18, BARN, SUGAR_MILL_PRODUCT), \
X(CANE_SUGAR, "caneSugar", 60, 20, 18, BARN, SUGAR_MILL_PRODUCT)

#define X(a, b, c, d, e, f, g) a
enum processed_item {
    PROCESSED_ITEM_TABLE
};
#undef X

//TODO animals. (a tad bit more complicated)

struct any_item {
    enum item_type type;
    union {
        enum field_crop field_item;
        enum tree_crop tree_item;
        enum special_item special_item;
        enum processed_item processed_item;
    };
};

const char *field_crop_enum_to_string(const enum field_crop type);
enum field_crop field_crop_string_to_enum(const char *type);
const char *tree_crop_enum_to_string(const enum tree_crop type);
enum tree_crop tree_crop_string_to_enum(const char *type);
enum storage get_storage_type_string(const char *string);
enum storage get_storage_type_field(const enum field_crop type);
enum storage get_storage_type_tree(const enum tree_crop type);
enum storage get_storage_type_special(const enum special_item type);
int tree_crop_buy_cost(const enum tree_crop item);
int tree_crop_sell_cost(const enum tree_crop item);
int field_crop_buy_cost(const enum field_crop item);
int field_crop_sell_cost(const enum field_crop item);
int special_item_buy_cost(const enum special_item item);
int special_item_sell_cost(const enum special_item item);
enum item_type get_product_type_string(const char *string);
enum item_type get_product_type_field(const enum field_crop type);
enum item_type get_product_type_tree(const enum tree_crop type);
enum item_type get_product_type_special(const enum special_item type);
enum special_item special_item_string_to_enum(const char *string);
const char *special_item_enum_to_string(const enum special_item item);
int item_buy_price_string(const char *string);
int item_sell_price_string(const char *string);
const char *processed_item_enum_to_string(const enum processed_item type);
enum processed_item processed_item_string_to_enum(const char *type);
int processed_item_buy_cost(const enum processed_item item);
int processed_item_sell_cost(const enum processed_item item);
enum storage get_storage_type_processed_item(const enum processed_item type);
enum item_type get_product_type_processed_item(const enum processed_item type);

const char *any_item_enum_to_string(const struct any_item item);
struct any_item any_item_string_to_enum(const char *string);
int any_item_buy_cost(const struct any_item item);
int any_item_sell_cost(const struct any_item item);
enum storage get_storage_type_any_item(const struct any_item item);

#endif /* TYPE_H */
