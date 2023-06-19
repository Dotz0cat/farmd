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

#include "type.h"

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=b
static const char *field_crop_strings[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type, maturity time
#define X(a, b, c, d, e, f, g, h) [a]=b
static const char *tree_crop_strings[] = {
    TREE_CROP_TABLE
};
#undef X

//enum, string, buy, sell, item_type, storage
#define X(a, b, c, d, e, f) [a]=b
static const char *special_item_strings[] = {
    SPECIAL_ITEM_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=b
static const char *processed_item_strings[] = {
    PROCESSED_ITEM_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=d
static const int field_crop_buy_cost_table[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type, maturity time
#define X(a, b, c, d, e, f, g, h) [a]=d
static const int tree_crop_buy_cost_table[] = {
    TREE_CROP_TABLE
};
#undef X

//enum, string, buy, sell, item_type, storage
#define X(a, b, c, d, e, f) [a]=c
static const int special_item_buy_cost_table[] = {
    SPECIAL_ITEM_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=d
static const int processed_item_buy_cost_table[] = {
    PROCESSED_ITEM_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=e
static const int field_crop_sell_cost_table[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type, maturity time
#define X(a, b, c, d, e, f, g, h) [a]=e
static const int tree_crop_sell_cost_table[] = {
    TREE_CROP_TABLE
};
#undef X

//enum, string, buy, sell, item_type, storage
#define X(a, b, c, d, e, f) [a]=d
static const int special_item_sell_cost_table[] = {
    SPECIAL_ITEM_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=e
static const int processed_item_sell_cost_table[] = {
    PROCESSED_ITEM_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=f
static const enum storage field_crop_storage[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type, maturity time
#define X(a, b, c, d, e, f, g, h) [a]=f
static const enum storage tree_crop_storage[] = {
    TREE_CROP_TABLE
};
#undef X

//enum, string, buy, sell, item_type, storage
#define X(a, b, c, d, e, f) [a]=f
static const enum storage special_item_storage[] = {
    SPECIAL_ITEM_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=f
static const enum storage processed_item_storage[] = {
    PROCESSED_ITEM_TABLE
};
#undef X

//yeah a little redundant, but its fast

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=g
static const enum item_type field_crop_type[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type, maturity time
#define X(a, b, c, d, e, f, g, h) [a]=g
static const enum item_type tree_crop_type[] = {
    TREE_CROP_TABLE
};
#undef X

//enum, string, buy, sell, item_type, storage
#define X(a, b, c, d, e, f) [a]=e
static const enum item_type special_item_type[] = {
    SPECIAL_ITEM_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=g
static const enum item_type processed_item_type[] = {
    PROCESSED_ITEM_TABLE
};
#undef X

const char *field_crop_enum_to_string(const enum field_crop type) {
    return field_crop_strings[type];
}

enum field_crop field_crop_string_to_enum(const char *type) {
    for (int i = 0; i < (int) (sizeof(field_crop_strings) / sizeof(field_crop_strings[0])); i++) {
        if (strcasecmp(type, field_crop_strings[i]) == 0) {
            return i;
        }
    }

    return NONE_FIELD;
}

const char *tree_crop_enum_to_string(const enum tree_crop type) {
    return tree_crop_strings[type];
}

enum tree_crop tree_crop_string_to_enum(const char *type) {
    for (int i = 0; i < (int) (sizeof(tree_crop_strings) / sizeof(tree_crop_strings[0])); i++) {
        if (strcasecmp(type, tree_crop_strings[i]) == 0) {
            return i;
        }
    }

    return NONE_TREE;
}
enum storage get_storage_type_string(const char *string) {
    if (strcasecmp(string, "none") == 0) {
        return NONE_STORAGE;
    }

    for (int i = 0; i < (int) (sizeof(field_crop_strings) / sizeof(field_crop_strings[0])); i++) {
        if (strcasecmp(string, field_crop_strings[i]) == 0) {
            return get_storage_type_field(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(tree_crop_strings) / sizeof(tree_crop_strings[0])); i++) {
        if (strcasecmp(string, tree_crop_strings[i]) == 0) {
            return get_storage_type_tree(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(special_item_strings) / sizeof(special_item_strings[0])); i++) {
        if (strcasecmp(string, special_item_strings[i]) == 0) {
            return get_storage_type_special(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(processed_item_strings) / sizeof(processed_item_strings[0])); i++) {
        if (strcasecmp(string, processed_item_strings[i]) == 0) {
            return get_storage_type_processed_item(i);
        }
    }

    return NONE_STORAGE;
}

enum storage get_storage_type_field(const enum field_crop type) {
    return field_crop_storage[type];
}

enum storage get_storage_type_tree(const enum tree_crop type) {
    return tree_crop_storage[type];
}

enum storage get_storage_type_special(const enum special_item type) {
    return special_item_storage[type];
}

int tree_crop_buy_cost(const enum tree_crop item) {
    return tree_crop_buy_cost_table[item];
}

int tree_crop_sell_cost(const enum tree_crop item) {
    return tree_crop_sell_cost_table[item];
}

int field_crop_buy_cost(const enum field_crop item) {
    return field_crop_buy_cost_table[item];
}

int field_crop_sell_cost(const enum field_crop item) {
    return field_crop_sell_cost_table[item];
}

int special_item_buy_cost(const enum special_item item) {
    return special_item_buy_cost_table[item];
}

int special_item_sell_cost(const enum special_item item) {
    return special_item_sell_cost_table[item];
}

enum item_type get_product_type_string(const char *string) {
    if (strcasecmp(string, "none") == 0) {
        return NONE_PRODUCT;
    }

    for (int i = 0; i < (int) (sizeof(field_crop_strings) / sizeof(field_crop_strings[0])); i++) {
        if (strcasecmp(string, field_crop_strings[i]) == 0) {
            return get_product_type_field(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(tree_crop_strings) / sizeof(tree_crop_strings[0])); i++) {
        if (strcasecmp(string, tree_crop_strings[i]) == 0) {
            return get_product_type_tree(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(special_item_strings) / sizeof(special_item_strings[0])); i++) {
        if (strcasecmp(string, special_item_strings[i]) == 0) {
            return get_product_type_special(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(processed_item_strings) / sizeof(processed_item_strings[0])); i++) {
        if (strcasecmp(string, processed_item_strings[i]) == 0) {
            return get_product_type_processed_item(i);
        }
    }

    return NONE_PRODUCT;
}

enum item_type get_product_type_field(const enum field_crop type) {
    return field_crop_type[type];
}

enum item_type get_product_type_tree(const enum tree_crop type) {
    return tree_crop_type[type];
}

enum item_type get_product_type_special(const enum special_item type) {
    return special_item_type[type];
}

enum special_item special_item_string_to_enum(const char *string) {
    for (int i = 0; i < (int) (sizeof(special_item_strings) / sizeof(special_item_strings[0])); i++) {
        if (strcasecmp(string, special_item_strings[i]) == 0) {
            return i;
        }
    }

    return NONE_SPECIAL;
}

const char *special_item_enum_to_string(const enum special_item item) {
    return special_item_strings[item];
}

int item_buy_price_string(const char *string) {
    if (strcasecmp(string, "none") == 0) {
        return 0;
    }

    for (int i = 0; i < (int) (sizeof(field_crop_strings) / sizeof(field_crop_strings[0])); i++) {
        if (strcasecmp(string, field_crop_strings[i]) == 0) {
            return field_crop_buy_cost(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(tree_crop_strings) / sizeof(tree_crop_strings[0])); i++) {
        if (strcasecmp(string, tree_crop_strings[i]) == 0) {
            return tree_crop_buy_cost(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(special_item_strings) / sizeof(special_item_strings[0])); i++) {
        if (strcasecmp(string, special_item_strings[i]) == 0) {
            return special_item_buy_cost(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(processed_item_strings) / sizeof(processed_item_strings[0])); i++) {
        if (strcasecmp(string, processed_item_strings[i]) == 0) {
            return processed_item_buy_cost(i);
        }
    }

    return 0;
}

int item_sell_price_string(const char *string) {
    if (strcasecmp(string, "none") == 0) {
        return 0;
    }

    for (int i = 0; i < (int) (sizeof(field_crop_strings) / sizeof(field_crop_strings[0])); i++) {
        if (strcasecmp(string, field_crop_strings[i]) == 0) {
            return field_crop_sell_cost(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(tree_crop_strings) / sizeof(tree_crop_strings[0])); i++) {
        if (strcasecmp(string, tree_crop_strings[i]) == 0) {
            return tree_crop_sell_cost(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(special_item_strings) / sizeof(special_item_strings[0])); i++) {
        if (strcasecmp(string, special_item_strings[i]) == 0) {
            return special_item_sell_cost(i);
        }
    }

    for (int i = 0; i < (int) (sizeof(processed_item_strings) / sizeof(processed_item_strings[0])); i++) {
        if (strcasecmp(string, processed_item_strings[i]) == 0) {
            return processed_item_sell_cost(i);
        }
    }

    return 0;
}

const char *processed_item_enum_to_string(const enum processed_item type) {
    return processed_item_strings[type];
}

enum processed_item processed_item_string_to_enum(const char *type) {
    for (int i = 0; i < (int) (sizeof(processed_item_strings) / sizeof(processed_item_strings[0])); i++) {
        if (strcasecmp(type, processed_item_strings[i]) == 0) {
            return i;
        }
    }

    return NONE_PROCESS;
}

int processed_item_buy_cost(const enum processed_item item) {
    return processed_item_buy_cost_table[item];
}

int processed_item_sell_cost(const enum processed_item item) {
    return processed_item_sell_cost_table[item];
}

enum storage get_storage_type_processed_item(const enum processed_item type) {
    return processed_item_storage[type];
}

enum item_type get_product_type_processed_item(const enum processed_item type) {
    return processed_item_type[type];
}

const char *any_item_enum_to_string(const struct any_item item) {
    switch (item.type) {
        case (FIELD_PRODUCT): {
            return field_crop_enum_to_string(item.field_item);
            break;
        }
        case (TREE_PRODUCT): {
            return tree_crop_enum_to_string(item.tree_item);
            break;
        }
        case (SPECIAL_PRODUCT): {
            return special_item_enum_to_string(item.special_item);
            break;
        }
        case (GRAIN_MILL_PRODUCT):
        case (SUGAR_MILL_PRODUCT): {
            return processed_item_enum_to_string(item.processed_item);
            break;
        }
        case (NONE_PRODUCT):
        case (OTHER_PRODUCT):
        default: {
            return "none";
        }
    }
}

struct any_item any_item_string_to_enum(const char *string) {
    struct any_item item;

    item.type = get_product_type_string(string);

    switch (item.type) {
        case (FIELD_PRODUCT): {
            item.field_item = field_crop_string_to_enum(string);
            break;
        }
        case (TREE_PRODUCT): {
            item.tree_item = tree_crop_string_to_enum(string);
            break;
        }
        case (SPECIAL_PRODUCT): {
            item.special_item = special_item_string_to_enum(string);
            break;
        }
        case (GRAIN_MILL_PRODUCT):
        case (SUGAR_MILL_PRODUCT): {
            item.processed_item = processed_item_string_to_enum(string);
            break;
        }
        case (NONE_PRODUCT):
        case (OTHER_PRODUCT):
        default: {
            break;
        }
    }

    return item;
}

int any_item_buy_cost(const struct any_item item) {
    switch (item.type) {
        case (FIELD_PRODUCT): {
            return field_crop_buy_cost(item.field_item);
            break;
        }
        case (TREE_PRODUCT): {
            return tree_crop_buy_cost(item.tree_item);
            break;
        }
        case (SPECIAL_PRODUCT): {
            return special_item_buy_cost(item.special_item);
            break;
        }
        case (GRAIN_MILL_PRODUCT):
        case (SUGAR_MILL_PRODUCT): {
            return processed_item_buy_cost(item.processed_item);
            break;
        }
        case (NONE_PRODUCT):
        case (OTHER_PRODUCT):
        default: {
            return 0;
        }
    }
}

int any_item_sell_cost(const struct any_item item) {
    switch (item.type) {
        case (FIELD_PRODUCT): {
            return field_crop_sell_cost(item.field_item);
            break;
        }
        case (TREE_PRODUCT): {
            return tree_crop_sell_cost(item.tree_item);
            break;
        }
        case (SPECIAL_PRODUCT): {
            return special_item_sell_cost(item.special_item);
            break;
        }
        case (GRAIN_MILL_PRODUCT):
        case (SUGAR_MILL_PRODUCT): {
            return processed_item_sell_cost(item.processed_item);
            break;
        }
        case (NONE_PRODUCT):
        case (OTHER_PRODUCT):
        default: {
            return 0;
        }
    }
}

enum storage get_storage_type_any_item(const struct any_item item) {
    switch (item.type) {
        case (FIELD_PRODUCT): {
            return get_storage_type_field(item.field_item);
            break;
        }
        case (TREE_PRODUCT): {
            return get_storage_type_tree(item.tree_item);
            break;
        }
        case (SPECIAL_PRODUCT): {
            return get_storage_type_special(item.special_item);
            break;
        }
        case (GRAIN_MILL_PRODUCT):
        case (SUGAR_MILL_PRODUCT): {
            return get_storage_type_processed_item(item.processed_item);
            break;
        }
        case (NONE_PRODUCT):
        case (OTHER_PRODUCT):
        default: {
            return NONE_STORAGE;
        }
    }
}
