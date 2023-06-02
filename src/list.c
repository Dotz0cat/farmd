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

#include "list.h"

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=b
const char* field_crop_strings[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=b
const char* tree_crop_strings[] = {
    TREE_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=d
const int field_crop_buy_cost_table[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=d
const int tree_crop_buy_cost_table[] = {
    TREE_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=e
const int field_crop_sell_cost_table[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=e
const int tree_crop_sell_cost_table[] = {
    TREE_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=f
const enum storage field_crop_storage[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=f
const enum storage tree_crop_storage[] = {
    TREE_CROP_TABLE
};
#undef X

//yeah a little redundant, but its fast

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=g
const enum item_type field_crop_type[] = {
    FIELD_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]=g
const enum item_type tree_crop_type[] = {
    TREE_CROP_TABLE
};
#undef X

static fields_list* add_field_to_list(fields_list* prev, int field_number) {
    if (prev == NULL) {
        fields_list* list = malloc(sizeof(fields_list));

        list->next = NULL;

        list->field_number = field_number;

        list->completion = 0;

        list->type = NONE_FIELD;

        list->event = NULL;

        return list;
    }
    else {
        fields_list* list = malloc(sizeof(fields_list));

        prev->next = list;

        list->next = NULL;

        list->field_number = field_number;

        list->completion = 0;

        list->type = NONE_FIELD;

        list->event = NULL;

        return list;
    }
}

fields_list* make_fields_list(const int number_of_fields) {
    if (number_of_fields == 0) {
        return NULL;
    }
    fields_list* list = NULL;

    list = add_field_to_list(list, 0);
    int i = 1;

    fields_list* add_head = list;

    for (; i < number_of_fields; i++) {
        add_head = add_field_to_list(add_head, i);
    }

    return list;
}

static fields_list* wind_fields_to_tail(fields_list* list) {
    while (list->next != NULL) {
        list = list->next;
    }

    return list;
}

int amend_fields_list(fields_list* head, int new_number) {
    int old_number = get_number_of_fields_list(head);

    if (new_number <= old_number) {
        //cannot sell fields
        return -1;
    }
    fields_list* list = head;

    list = wind_fields_to_tail(list);

    for (int i = old_number - 1; i < new_number; i++) {
        add_field_to_list(list, i);
    }

    return 0;
}

int get_number_of_fields_list(fields_list* head) {
    int number_of_fields = 0;
    fields_list* count_head = head;
    do {
        number_of_fields++;
        count_head = count_head->next;
    } while(count_head != NULL);

    return number_of_fields;
}

void set_field_event_pointer(fields_list* list, void* event) {
    list->event = event;
} 

const char* field_crop_enum_to_string(const enum field_crop type) {
    return field_crop_strings[type];
}

enum field_crop field_crop_string_to_enum(const char* type) {
    for (int i = 0; i < (int) sizeof(field_crop_strings); i++) {
        if (strcasecmp(type, field_crop_strings[i]) == 0) {
            return i;
        }
    }

    return NONE_FIELD;
}

static trees_list* add_tree_to_list(trees_list* prev, int tree_number) {
    if (prev == NULL) {
        trees_list* new = malloc(sizeof(trees_list));

        new->type = NONE_TREE;

        new->next = NULL;

        new->event = NULL;

        new->tree_number = tree_number;

        return new;
    }
    else {
        trees_list* new = malloc(sizeof(trees_list));

        new->type = NONE_TREE;

        new->next = NULL;

        new->event = NULL;

        new->tree_number = tree_number;

        prev->next = new;

        return new;
    }
}

trees_list* make_trees_list(const int number_of_trees) {
    if (number_of_trees == 0) {
        return NULL;
    }

    trees_list* list = NULL;

    list = add_tree_to_list(list, 0);
    int i = 1;

    trees_list* add_head = list;

    for (; i < number_of_trees; i++) {
        add_head = add_tree_to_list(add_head, i);
    }

    return list;
}

static trees_list* wind_trees_to_tail(trees_list* head) {
    trees_list* list = head;

    while (list->next != NULL) {
        list = list->next;
    }

    return list;
}

int amend_trees_list(trees_list* head, const int new_number) {
    int old_number = get_number_of_trees_list(head);

    if (new_number <= old_number) {
        return -1;
    }

    trees_list* list = head;

    list = wind_trees_to_tail(list);

    for (int i = old_number - 1; i < new_number; i++) {
        list = add_tree_to_list(list, i);
    }

    return 0;
}

int get_number_of_trees_list(trees_list* head) {
    int count = 0;
    trees_list* count_head = head;

    do {
        count++;
        count_head = count_head->next;
    } while(count_head != NULL);

    return count;
}

void set_trees_event_pointer(trees_list* node, void* event) {
    node->event = event;
}

const char* tree_crop_enum_to_string(const enum tree_crop type) {
    return tree_crop_strings[type];
}

enum tree_crop tree_crop_string_to_enum(const char* type) {
    for (int i = 0; i < (int) sizeof(tree_crop_strings); i++) {
        if (strcasecmp(type, tree_crop_strings[i]) == 0) {
            return i;
        }
    }

    return NONE_TREE;
}

enum storage get_storage_type_string(const char* string) {
    if (strcasecmp(string, "none") == 0) {
        return NONE_STORAGE;
    }

    for (int i = 0; i < (int) sizeof(field_crop_strings); i++) {
        if (strcasecmp(string, field_crop_strings[i]) == 0) {
            return get_storage_type_field(i);
        }
    }

    for (int i = 0; i < (int) sizeof(tree_crop_strings); i++) {
        if (strcasecmp(string, tree_crop_strings[i]) == 0) {
            return get_storage_type_tree(i);
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

int tree_crop_buy_cost(enum tree_crop item) {
    return tree_crop_buy_cost_table[item];
}

int tree_crop_sell_cost(enum tree_crop item) {
    return tree_crop_sell_cost_table[item];
}

int field_crop_buy_cost(enum field_crop item) {
    return field_crop_buy_cost_table[item];
}

int field_crop_sell_cost(enum field_crop item) {
    return field_crop_sell_cost_table[item];
}

enum item_type get_product_type_string(const char* string) {
    if (strcasecmp(string, "none") == 0) {
        return NONE_PRODUCT;
    }

    for (int i = 0; i < (int) sizeof(field_crop_strings); i++) {
        if (strcasecmp(string, field_crop_strings[i]) == 0) {
            return get_product_type_field(i);
        }
    }

    for (int i = 0; i < (int) sizeof(tree_crop_strings); i++) {
        if (strcasecmp(string, tree_crop_strings[i]) == 0) {
            return get_product_type_tree(i);
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
