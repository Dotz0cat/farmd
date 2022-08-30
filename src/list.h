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

#define FIELD_CROP_TABLE \
X(NONE_FIELD, "none", 0), \
X(WHEAT, "wheat", 30), \
X(CORN, "corn", 60), \
X(POTATOS, "potatos", 60), \
X(BEETS, "beets", 60), \
X(SUGARCANE, "sugarcane", 60), \
X(TURNIPS, "turnips", 60), \
X(TOMATOS, "tomatos", 60), \
X(CUCUMBERS, "cucumbers", 60), \
X(OKRA, "okra", 60)

#define X(a, b, c) a
enum field_crop {
    FIELD_CROP_TABLE
};
#undef X

#define TREE_CROP_TABLE \
X(NONE_TREE, "none", 0), \
X(PEARS, "pears", 60), \
X(APPLES, "apples", 60), \
X(ORANGES, "oranges", 60), \
X(PEACHES, "peaches", 60)

#define X(a,b, c) a
enum tree_crop {
    TREE_CROP_TABLE
};
#undef X

enum animal_products {
    MILK,
    EGGS
};

typedef struct _fields_list fields_list;

struct _fields_list {
    int field_number;

    enum field_crop type;

    int completion;

    //may not be used.
    void* event;

    void* next;
};

typedef struct _trees_list trees_list;

struct _trees_list {
    int tree_number;

    enum tree_crop type;

    int completion;

    //may not be used.
    void* event;

    void* next;
};

//TODO animals. (a tad bit more complicated)

static fields_list* add_field_to_list(fields_list* prev, int field_number);
fields_list* make_fields_list(int number_of_fields);
fields_list* wind_to_tail(fields_list* list);
int amend_fields_list(fields_list* head, int new_number);
int get_number_of_fields_list(fields_list* head);
void set_event_pointer(fields_list* list, void* event);
const char* field_crop_enum_to_string(enum field_crop type);
enum field_crop field_crop_string_to_enum(const char* type);

#endif /* LIST_H */
