/*
Copyright 2022-2023 Dotz0cat

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
#include "type.h"

typedef struct _fields_list fields_list;

struct _fields_list {
    int field_number;

    enum field_crop type;

    int completion;

    void *event;

    void *next;
};

typedef struct _trees_list trees_list;

struct _trees_list {
    int tree_number;

    enum tree_crop type;

    int maturity;

    int completion;

    void *event;

    void *next;
};

typedef struct _slot_list slot_list;

struct _slot_list {
    int slot_number;

    enum processed_item type;

    int completion;

    unsigned long long mask;

    void *event;

    void *next;
};

typedef struct _queue_list queue_list;

struct _queue_list {
    int queue_number;

    unsigned long long activity;
    unsigned long long fill_state;
    unsigned long long complete;

    unsigned long long slot_mask;

    slot_list *slots;

    slot_list *new_task_ptr;
    slot_list *collect_ptr;
    slot_list *active_ptr;

    void *next;
};

fields_list *make_fields_list(const int number_of_fields);
int amend_fields_list(fields_list *head, int new_number);
int get_number_of_fields_list(fields_list *head);
void set_field_event_pointer(fields_list *list, void *event);

trees_list *make_trees_list(const int number_of_trees);
int amend_trees_list(trees_list *head, const int new_number);
int get_number_of_trees_list(trees_list *head);
void set_trees_event_pointer(trees_list *node, void *event);

slot_list *make_slot_list(const int number_of_slots);
int amend_slot_list(slot_list *head, const int new_number);
int get_number_of_slot_list(slot_list *head);

queue_list *make_queue_list(const int number_of_queues);
int amend_queue_list(queue_list *head, const int new_number);
int get_number_of_queue_list(queue_list *head);

#endif /* LIST_H */
