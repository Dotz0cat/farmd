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

#define X(a, b, c) [a]=b
const char* field_crop_strings[] = {
    FIELD_CROP_TABLE
};
#undef X

#define X(a, b, c) [a]=b
const char* tree_crop_strings[] = {
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

fields_list* make_fields_list(int number_of_fields) {
    if (number_of_fields == 0) {
        return NULL;
    }
    fields_list* list = NULL;

    list = add_field_to_list(list, 0);
    int i = 1;

    for (; i < number_of_fields; i++) {
        add_field_to_list(list, i);
    }

    return list;
}

fields_list* wind_to_tail(fields_list* list) {
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

    list = wind_to_tail(list);

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
    } while(count_head->next != NULL);

    return number_of_fields;
}

void set_event_pointer(fields_list* list, void* event) {
    list->event = event;
} 

const char* field_crop_enum_to_string(enum field_crop type) {
    return field_crop_strings[type];
}

enum field_crop field_crop_string_to_enum(const char* type) {
    for (int i = 0; i < sizeof(field_crop_strings); i++) {
        if (strcmp(type, field_crop_strings[i]) == 0) {
            return i;
        }
    }

    return NONE_FIELD;
}
