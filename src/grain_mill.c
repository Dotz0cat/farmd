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

#include "grain_mill_private.h"

void populate_grain_mill(sqlite3 *db, queue_list **queue, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg)) {
    int number_of_queues = get_grain_mill_meta_property(db, "Queues");

    *queue = make_queue_list(number_of_queues);

    if (number_of_queues <= 0) {
        return;
    }
    queue_list *list = *queue;
    for (int i = 0; i < number_of_queues; i++) {
        setup_grain_mill_queue(db, &list->slots, list->queue_number, base, cb);
    }

}

static void setup_grain_mill_queue(sqlite3 *db, slot_list **slots, int queue, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg)) {
    int char_count = snprintf(NULL, 0, "SlotsInQueue%i", queue);

    if (char_count <= 0) {
        abort();
    }

    char *property = malloc(char_count + 1U);

    if (property == NULL) {
        abort();
    }

    snprintf(property, char_count + 1U, "SlotsInQueue%i", queue);

    int number_of_slots = get_grain_mill_meta_property(db, property);

    free(property);

    *slots = make_slot_list(number_of_slots);

    if (number_of_slots <= 0) {
        return;
    }
    slot_list *list = *slots;
    for (int i = 0; i < number_of_slots; i++) {
        char *slot_type = get_grain_mill_type(db, queue, i);
        list->type = processed_item_string_to_enum(slot_type);
        free(slot_type);

        if (list->type != NONE_PROCESS) {
            setup_grain_mill_completion(db, list, queue, base, cb);
        }
        list = list->next;
    }
}

static void setup_grain_mill_completion(sqlite3 *db, slot_list *list, int queue, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg)) {
    if (get_grain_mill_completion(db, queue, list->slot_number) == 0) {
        time_t now = time(NULL);
        time_t start_time_from_db = get_grain_mill_start_time(db, queue, list->slot_number);
        time_t end_time_from_db = get_grain_mill_end_time(db, queue, list->slot_number);

        if (end_time_from_db <= now) {
            list->completion = 1;
            set_grain_mill_completion(db, queue, list->slot_number, 1);
        }
        else if (start_time_from_db > now) {
            //start time is in the future
            list->completion = 0;
        }
        else {
            //make an event as it has started and not finshed
            struct timeval tv;
            tv.tv_sec = end_time_from_db - now;
            tv.tv_usec = 0;

            struct box_for_list_and_db *box = malloc(sizeof(struct box_for_list_and_db));
            box->list = list;
            box->db = db;
            list->event = event_new(base, -1, 0, cb, box);

            event_add(list->event, &tv);
        }
    }
    else {
        list->completion = 1;
    }
}

void ping_grain_mill(sqlite3 *db) {
    int number_of_queues = get_grain_mill_meta_property(db, "Queues");

    if (number_of_queues <= 0) {
        return;
    }

    for (int i = 0; i < number_of_queues; i++) {
        int char_count = snprintf(NULL, 0, "SlotsInQueue%i", i);

        if (char_count <= 0) {
            abort();
        }

        char *property = malloc(char_count + 1U);

        if (property == NULL) {
            abort();
        }

        snprintf(property, char_count + 1U, "SlotsInQueue%i", i);

        int number_of_slots = get_grain_mill_meta_property(db, property);

        free(property);

        for (int j = 0; j < number_of_slots; j++) {
            char *slot_type = get_grain_mill_type(db, i, j);
            enum processed_item type = processed_item_string_to_enum(slot_type);
            free(slot_type);

            if (type != NONE_PROCESS) {
                if (get_grain_mill_completion(db, i, j) == 0) {
                    time_t now = time(NULL);
                    time_t end_time_from_db = get_grain_mill_end_time(db, i, j);

                    if (end_time_from_db <= now) {
                        set_grain_mill_completion(db, i, j, 1);
                    }
                }
            }
        }
    }
}

void free_queue_list(queue_list **head) {
    if (*head != NULL) {
        queue_list *list = *head;
        while(list != NULL) {
            free_slot_list(&list->slots);
            queue_list *temp = list;
            list = list->next;
            free(temp);
        }
    }
    *head = NULL;
}

void free_slot_list(slot_list **head) {
    if (*head != NULL) {
        slot_list *list = *head;
        while (list != NULL) {
            if (list->event != NULL) {
                event_del(list->event);
                void *temp;
                if ((temp = event_get_callback_arg(list->event)) != NULL) {
                    free(temp);
                }
                event_free(list->event);
            }
            slot_list *temp = list;
            list = list->next;
            free(temp);
        }

    }
    *head = NULL;
}
