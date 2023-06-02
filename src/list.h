/*
Copyright 2022 Dotz0cat

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

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
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

//enum, string, time, buy, sell, storage, item_type
#define TREE_CROP_TABLE \
X(NONE_TREE, "none", 0, 0, 0, NONE_STORAGE, NONE_PRODUCT), \
X(PEARS, "pears", 60, 10, 8, BARN, TREE_PRODUCT), \
X(APPLES, "apples", 60, 10, 8, BARN, TREE_PRODUCT), \
X(ORANGES, "oranges", 60, 10, 8, BARN, TREE_PRODUCT), \
X(PEACHES, "peaches", 60, 10, 8, BARN, TREE_PRODUCT)

#define X(a, b, c, d, e, f, g) a
enum tree_crop {
    TREE_CROP_TABLE
};
#undef X

typedef struct _fields_list fields_list;

struct _fields_list {
    int field_number;

    enum field_crop type;

    int completion;

    void* event;

    void* next;
};

typedef struct _trees_list trees_list;

struct _trees_list {
    int tree_number;

    enum tree_crop type;

    int completion;

    void* event;

    void* next;
};

//TODO animals. (a tad bit more complicated)

static fields_list* add_field_to_list(fields_list* prev, int field_number);
fields_list* make_fields_list(const int number_of_fields);
static fields_list* wind_fields_to_tail(fields_list* list);
int amend_fields_list(fields_list* head, int new_number);
int get_number_of_fields_list(fields_list* head);
void set_field_event_pointer(fields_list* list, void* event);
const char* field_crop_enum_to_string(const enum field_crop type);
enum field_crop field_crop_string_to_enum(const char* type);

static trees_list* add_tree_to_list(trees_list* prev, int tree_number);
trees_list* make_trees_list(const int number_of_trees);
static trees_list* wind_trees_to_tail(trees_list* head);
int amend_trees_list(trees_list* head, const int new_number);
int get_number_of_trees_list(trees_list* head);
void set_trees_event_pointer(trees_list* node, void* event);
const char* tree_crop_enum_to_string(const enum tree_crop type);
enum tree_crop tree_crop_string_to_enum(const char* type);

enum storage get_storage_type_string(const char* string);
enum storage get_storage_type_field(const enum field_crop type);
enum storage get_storage_type_tree(const enum tree_crop type);
int tree_crop_buy_cost(enum tree_crop item);
int tree_crop_sell_cost(enum tree_crop item);
int field_crop_buy_cost(enum field_crop item);
int field_crop_sell_cost(enum field_crop item);
enum item_type get_product_type_string(const char* string);
enum item_type get_product_type_field(const enum field_crop type);
enum item_type get_product_type_tree(const enum tree_crop type);

#endif /* LIST_H */
