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

#include "list_private.h"

static fields_list *add_field_to_list(fields_list *prev, int field_number) {
    fields_list *list = malloc(sizeof(fields_list));

    list->next = NULL;

    list->field_number = field_number;

    list->completion = 0;

    list->type = NONE_FIELD;

    list->event = NULL;

    if (prev != NULL) {
        prev->next = list;
    }

    return list;
}

fields_list *make_fields_list(const int number_of_fields) {
    if (number_of_fields == 0) {
        return NULL;
    }
    fields_list *list = NULL;

    list = add_field_to_list(list, 0);
    int i = 1;

    fields_list *add_head = list;

    for (; i < number_of_fields; i++) {
        add_head = add_field_to_list(add_head, i);
    }

    return list;
}

static fields_list *wind_fields_to_tail(fields_list *list) {
    while (list->next != NULL) {
        list = list->next;
    }

    return list;
}

int amend_fields_list(fields_list *head, int new_number) {
    int old_number = get_number_of_fields_list(head);

    if (new_number <= old_number || head == NULL) {
        return -1;
    }
    fields_list *list = head;

    list = wind_fields_to_tail(list);

    if (list == NULL) {
        return -1;
    }

    for (int i = old_number; i < new_number; i++) {
        add_field_to_list(list, i);
    }

    return 0;
}

int get_number_of_fields_list(fields_list *head) {
    int number_of_fields = 0;
    fields_list *count_head = head;
    do {
        number_of_fields++;
        count_head = count_head->next;
    } while(count_head != NULL);

    return number_of_fields;
}

void set_field_event_pointer(fields_list *list, void *event) {
    list->event = event;
}

static trees_list *add_tree_to_list(trees_list *prev, int tree_number) {
    trees_list *new = malloc(sizeof(trees_list));

    new->type = NONE_TREE;

    new->next = NULL;

    new->event = NULL;

    new->tree_number = tree_number;

    new->completion = 0;

    new->maturity = 0;

    if (prev != NULL) {
        prev->next = new;
    }

    return new;
}

trees_list *make_trees_list(const int number_of_trees) {
    if (number_of_trees == 0) {
        return NULL;
    }

    trees_list *list = NULL;

    list = add_tree_to_list(list, 0);
    int i = 1;

    trees_list *add_head = list;

    for (; i < number_of_trees; i++) {
        add_head = add_tree_to_list(add_head, i);
    }

    return list;
}

static trees_list *wind_trees_to_tail(trees_list *head) {
    trees_list *list = head;

    while (list->next != NULL) {
        list = list->next;
    }

    return list;
}

int amend_trees_list(trees_list *head, const int new_number) {
    int old_number = get_number_of_trees_list(head);

    if (new_number <= old_number || head == NULL) {
        return -1;
    }

    trees_list *list = head;

    list = wind_trees_to_tail(list);

    if (list == NULL) {
        return -1;
    }

    for (int i = old_number; i < new_number; i++) {
        list = add_tree_to_list(list, i);
    }

    return 0;
}

int get_number_of_trees_list(trees_list *head) {
    int count = 0;
    trees_list *count_head = head;

    do {
        count++;
        count_head = count_head->next;
    } while(count_head != NULL);

    return count;
}

void set_trees_event_pointer(trees_list *node, void *event) {
    node->event = event;
}

static slot_list *add_slot_to_list(slot_list *prev, int slot_number) {
    slot_list *new = malloc(sizeof(slot_list));

    new->type = NONE_PROCESS;

    new->next = NULL;

    new->mask = 0ull;
    new->mask |= 1 << slot_number;

    new->event = NULL;

    new->slot_number = slot_number;

    if (prev != NULL) {
        prev->next = new;
    }

    return new;
}

slot_list *make_slot_list(const int number_of_slots) {
    if (number_of_slots == 0 || number_of_slots > 64) {
        return NULL;
    }

    slot_list *list = NULL;

    list = add_slot_to_list(list, 0);
    int i = 1;

    slot_list *add_head = list;

    for (; i < number_of_slots; i++) {
        add_head = add_slot_to_list(add_head, i);
    }

    //make circular
    add_head->next = list;

    return list;
}

static slot_list *wind_slots_to_tail(slot_list *head) {
    slot_list *list = head;

    while (list->next != NULL && list->next != head) {
        list = list->next;
    }

    return list;
}

int amend_slot_list(slot_list *head, const int new_number) {
    int old_number = get_number_of_slot_list(head);

    if (new_number <= old_number || head == NULL || new_number > 64) {
        return -1;
    }

    slot_list *list = head;

    list = wind_slots_to_tail(list);

    if (list == NULL) {
        return -1;
    }

    for (int i = old_number; i < new_number; i++) {
        list = add_slot_to_list(list, i);
    }

    //relink
    list->next = head;

    return 0;
}

int get_number_of_slot_list(slot_list *head) {
    int count = 0;
    slot_list *count_head = head;

    do {
        count++;
        count_head = count_head->next;
    } while(count_head != NULL && count_head != head);

    return count;
}

static queue_list *add_queue_to_list(queue_list *prev, int queue_number) {
   queue_list *new = malloc(sizeof(queue_list));

    new->next = NULL;

    new->activity = 0ull;
    new->fill_state = 0ull;
    new->complete = 0ull;
    new->slot_mask = 0ull;

    new->slots = NULL;
    new->new_task_ptr = NULL; //front
    new->collect_ptr = NULL; //rear
    new->active_ptr = NULL; //middle

    new->queue_number = queue_number;

    if (prev != NULL) {
        prev->next = new;
    }

    return new;
}

queue_list *make_queue_list(const int number_of_queues) {
    if (number_of_queues == 0) {
        return NULL;
    }

    queue_list *list = NULL;

    list = add_queue_to_list(list, 0);
    int i = 1;

    queue_list *add_head = list;

    for (; i < number_of_queues; i++) {
        add_head = add_queue_to_list(add_head, i);
    }

    return list;
}

static queue_list *wind_queue_to_tail(queue_list *head) {
    queue_list *list = head;

    while (list->next != NULL) {
        list = list->next;
    }

    return list;
}

int amend_queue_list(queue_list *head, const int new_number) {
    int old_number = get_number_of_queue_list(head);

    if (new_number <= old_number || head == NULL) {
        return -1;
    }

    queue_list *list = head;

    list = wind_queue_to_tail(list);

    if (list == NULL) {
        return -1;
    }

    for (int i = old_number; i < new_number; i++) {
        list = add_queue_to_list(list, i);
    }

    return 0;
}

int get_number_of_queue_list(queue_list *head) {
    int count = 0;
    queue_list *count_head = head;

    do {
        count++;
        count_head = count_head->next;
    } while(count_head != NULL);

    return count;
}
