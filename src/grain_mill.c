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
        setup_grain_mill_queue(db, &list->slots, list, base, cb);

        if (list->collect_ptr == NULL) {
            list->collect_ptr = list->slots;
        }
        if (list->active_ptr == NULL) {
            list->active_ptr = list->slots;
        }
        if (list->new_task_ptr == NULL) {
            list->new_task_ptr = list->slots;
        }
    }

}

static void setup_grain_mill_queue(sqlite3 *db, slot_list **slots, queue_list *queue, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg)) {
    int char_count = snprintf(NULL, 0, "SlotsInQueue%i", queue->queue_number);

    if (char_count <= 0) {
        abort();
    }

    char *property = malloc(char_count + 1U);

    if (property == NULL) {
        abort();
    }

    snprintf(property, char_count + 1U, "SlotsInQueue%i", queue->queue_number);

    int number_of_slots = get_grain_mill_meta_property(db, property);

    free(property);

    *slots = make_slot_list(number_of_slots);

    if (number_of_slots <= 0 || number_of_slots > 64) {
        return;
    }
    slot_list *list = *slots;
    int first_filled = 0;
    for (int i = 0; i < number_of_slots; i++) {
        char *slot_type = get_grain_mill_type(db, queue->queue_number, i);
        list->type = processed_item_string_to_enum(slot_type);
        free(slot_type);

        queue->slot_mask |= list->mask;

        if (list->type != NONE_PROCESS) {
            queue->fill_state |= list->mask;
            setup_grain_mill_completion(db, list, queue, base, cb);
            if (first_filled == 0) {
                queue->collect_ptr = list;
            }
            first_filled++;
            //set each time so whenever it is last set it will be the last
            queue->new_task_ptr = list;
        }
        list = list->next;
    }
}

static void setup_grain_mill_completion(sqlite3 *db, slot_list *list, queue_list *queue, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg)) {
    if (get_grain_mill_completion(db, queue->queue_number, list->slot_number) == 0) {
        time_t now = time(NULL);
        time_t start_time_from_db = get_grain_mill_start_time(db, queue->queue_number, list->slot_number);
        time_t end_time_from_db = get_grain_mill_end_time(db, queue->queue_number, list->slot_number);

        if (end_time_from_db <= now) {
            list->completion = 1;
            queue->complete |= list->mask;
            set_grain_mill_completion(db, queue->queue_number, list->slot_number, 1);
        }
        else if (start_time_from_db > now) {
            //start time is in the future
            list->completion = 0;
            //make the event
            struct box_for_list_and_db *box = malloc(sizeof(struct box_for_list_and_db));
            box->list = queue;
            box->db = db;

            list->event = event_new(base, -1, 0, cb, box);
        }
        else {
            //make an event as it has started and not finshed
            //only if there is no activity on the queue
            if (queue->activity == 0) {
                struct timeval tv;
                tv.tv_sec = end_time_from_db - now;
                tv.tv_usec = 0;

                struct box_for_list_and_db *box = malloc(sizeof(struct box_for_list_and_db));
                box->list = queue;
                box->db = db;
                list->event = event_new(base, -1, 0, cb, box);

                event_add(list->event, &tv);
                queue->activity |= list->mask;
                queue->active_ptr = list;
            }
            else {
                //just make the event
                struct box_for_list_and_db *box = malloc(sizeof(struct box_for_list_and_db));
                box->list = queue;
                box->db = db;

                list->event = event_new(base, -1, 0, cb, box);
            }
        }
    }
    else {
        list->completion = 1;
        queue->complete |= list->mask;
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
            if (list->slots != NULL) {
                free_slot_list(&list->slots);
            }
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
        do {
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
        } while (list != NULL && list != *head);
    }
    *head = NULL;
}

struct evbuffer *buy_grain_mill(sqlite3 *db, queue_list **queue, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    if (get_grain_mill_bought_status(db) != 0 || get_grain_mill_meta_property(db, "Level") != 0) {
        evbuffer_add_printf(returnbuffer, "already own grain mill\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //check eligbility
    int current_farm_level = get_level(db);

    if (get_skill_status(db, "GrainMill") <= 0) {
        evbuffer_add_printf(returnbuffer, "do not own skill for grain mill\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    if (current_farm_level < GRAIN_MILL_LEVEL_COST) {
        evbuffer_add_printf(returnbuffer, "not high enough level for grain mill\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    if (money_check(db, GRAIN_MILL_BUY_COST) == 0) {
        evbuffer_add_printf(returnbuffer, "not enough money for grain mill\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    switch(subtract_money(db, GRAIN_MILL_BUY_COST)) {
        case (NO_MONEY_ERROR): {
            break;
        }
        case (NOT_ENOUGH): {
            evbuffer_add_printf(returnbuffer, "not enough money for grain mill\r\n");
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

    //update db
    if (update_meta(db, 1, "GrainMill")) {
        evbuffer_add_printf(returnbuffer, "error updating meta\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    if (update_grain_mill_meta_property(db, "Level", 1)) {
        evbuffer_add_printf(returnbuffer, "error updating grain mill meta\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    if (update_grain_mill_meta_property(db, "Queues", 1)) {
        evbuffer_add_printf(returnbuffer, "error updating grain mill meta\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    if (update_grain_mill_meta_property(db, "SlotsInQueue0", 3)) {
        evbuffer_add_printf(returnbuffer, "error updating grain mill meta\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //make the lists
    int number_of_queues = get_grain_mill_meta_property(db, "Queues");
    *queue = make_queue_list(number_of_queues);

    queue_list *list = *queue;
    for (int i = 0; i < number_of_queues; i++) {
        if (list == NULL) {
            break;
        }
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

        list->slots = make_slot_list(number_of_slots);

        slot_list *list_of_slots = list->slots;
        do {
            list->slot_mask |= list_of_slots->mask;
            list_of_slots = list_of_slots->next;
        } while (list_of_slots != list->slots);

        //set pointers
        list->collect_ptr = list->slots;
        list->active_ptr = list->slots;
        list->new_task_ptr = list->slots;

        for (int j = 0; j < number_of_slots; j++) {
            if (add_slot_to_grain_mill(db, i, j) != 0) {
                evbuffer_add_printf(returnbuffer, "error adding slot to grain mill in db\r\n");
                SET_CODE_INTERNAL_ERROR(code)
                return returnbuffer;
            }
        }

        list = list->next;
    }

    evbuffer_add_printf(returnbuffer, "Sucessfully bought grain mill\r\n");
    SET_CODE_OK(code)
    return returnbuffer;
}

struct evbuffer *get_grain_mill_buy_cost(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    evbuffer_add_printf(returnbuffer, "grain mill cost:\r\n");

    if (get_grain_mill_bought_status(db) != 0 || get_grain_mill_meta_property(db, "Level") != 0) {
        evbuffer_add_printf(returnbuffer, "already own grain mill\r\n");
    }

    evbuffer_add_printf(returnbuffer, "Level: %i\r\nMoney: %i\r\nGrainMill skill\r\n", GRAIN_MILL_LEVEL_COST, GRAIN_MILL_BUY_COST);
    SET_CODE_OK(code)
    return returnbuffer;
}

struct evbuffer *upgrade_grain_mill(sqlite3 *db, queue_list **queue, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    if (get_grain_mill_bought_status(db) == 0 || get_grain_mill_meta_property(db, "Level") == 0) {
        evbuffer_add_printf(returnbuffer, "don't own grain mill\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //check eligbitly 
    //1 level per 5 farm levels after level 5
    int current_farm_level = get_level(db);

    int current_grain_mill_level = get_grain_mill_meta_property(db, "Level");

    current_farm_level = current_farm_level - 5;

    if (current_grain_mill_level > (current_farm_level / 5)) {
        evbuffer_add_printf(returnbuffer, "insuffecent farm level to upgrade\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //calculate items
    int amount = 3 + (current_grain_mill_level / 2);
    int money_amount = 1500 + (500 * current_grain_mill_level);

    //lookup item
    const char *upgrade_item = special_item_enum_to_string(GRAIN_MILL_UPGRADE_ITEM);

    //check
    if (items_available(db, upgrade_item, amount) == 0) {
        evbuffer_add_printf(returnbuffer, "insuffecent items to upgrade\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //subtract items
    switch (subtract_money(db, money_amount)) {
        case (NO_MONEY_ERROR): {
            break;
        }
        case (ERROR_UPDATING): {
            evbuffer_add_printf(returnbuffer, "could not subtract money\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
        }
        case (NOT_ENOUGH): {
            evbuffer_add_printf(returnbuffer, "insuffecent money to upgrade\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
        }
    }

    switch (remove_from_storage(db, upgrade_item, amount)) {
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
            evbuffer_add_printf(returnbuffer, "not enough %s in barn\r\n", upgrade_item);
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
            evbuffer_add_printf(returnbuffer, "not enough %s in silo\r\n", upgrade_item);
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

    //update db
    if (update_grain_mill_meta_property(db, "Level", 1)) {
        evbuffer_add_printf(returnbuffer, "error updating grain mill meta\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //add a new queue each level
    //each new queue has 3 slots
    int new_slots = 3;
    int current_queues = get_grain_mill_meta_property(db, "Queues");

    //queue numbers start at 0
    int char_count = snprintf(NULL, 0, "SlotsInQueue%i", current_queues);

    if (char_count <= 0) {
        abort();
    }

    char *property = malloc(char_count + 1U);

    if (property == NULL) {
        abort();
    }

    snprintf(property, char_count + 1U, "SlotsInQueue%i", current_queues);

    if (update_grain_mill_meta_property(db, "Queues", 1)) {
        evbuffer_add_printf(returnbuffer, "error updating grain mill meta\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    if (update_grain_mill_meta_property(db, property, new_slots)) {
        evbuffer_add_printf(returnbuffer, "error updating grain mill meta\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    for (int i = 0; i < new_slots; i++) {
        if (add_slot_to_grain_mill(db, current_queues, i)) {
            evbuffer_add_printf(returnbuffer, "error adding to grain mill meta\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
        }
    }
    free(property);

    //setup queue now
    if (amend_queue_list(*queue, current_queues) != 0) {
        evbuffer_add_printf(returnbuffer, "error amending queue list\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    queue_list *list = *queue;
    while (list->next != NULL && list->slots != NULL) {
        list = list->next;
    }

    //now at the end
    list->slots = make_slot_list(new_slots);

    slot_list *slots = list->slots;
    //make slot mask
    do {
        list->slot_mask |= slots->mask;
        slots = slots->next;
    } while(slots != list->slots);

    //set pointers
    list->collect_ptr = list->slots;
    list->active_ptr = list->slots;
    list->new_task_ptr = list->slots;

    evbuffer_add_printf(returnbuffer, "Sucessfully upgraded grain mill\r\n");
    SET_CODE_OK(code)
    return returnbuffer;
}

struct evbuffer *get_grain_mill_upgrade_cost(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    //get current
    int current_level = get_grain_mill_meta_property(db, "Level");

    int amount = 3 + (current_level / 2);
    int money_amount = 1500 + (500 * current_level);

    const char *item_name = special_item_enum_to_string(GRAIN_MILL_UPGRADE_ITEM);

    evbuffer_add_printf(returnbuffer, "Costs:\r\n%s: %d\r\nMoney: %d\r\n", item_name, amount, money_amount);
    SET_CODE_OK(code)
    return returnbuffer;
}

struct evbuffer *get_grain_mill_next_level_stats(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    int current_level = get_grain_mill_meta_property(db, "Level");
    int current_queues = get_grain_mill_meta_property(db, "Queues");
    current_queues++;
    current_level++;

    evbuffer_add_printf(returnbuffer, "Stats:\r\nLevel: %d\r\nQueues: %d\r\n", current_level, current_queues);
    SET_CODE_OK(code)
    return returnbuffer;
}

struct evbuffer *collect_grain_mill_procducts(sqlite3 *db, queue_list *queue, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    //get queue list to incriment through
    queue_list *q_list = queue;
    while (q_list != NULL) {
        //check the masks. if there is none compelete it will move to next queue
        if ((q_list->complete & q_list->slot_mask) == 0) {
            q_list = q_list->next;
            continue;
        }
        //get slot list at collect_ptr
        slot_list *s_list = q_list->collect_ptr;

        //test to make sure it is not equal to active_ptr
        if (s_list == q_list->active_ptr) {
            //if equal to 0 then it is full
            //if there is no activity then don't go to next queue
            if ((q_list->fill_state ^ q_list->slot_mask) != 0 || (q_list->activity & q_list->slot_mask) != 0) {
                q_list = q_list->next;
                continue;
            }
        }

        while (s_list != q_list->active_ptr || (q_list->fill_state ^ q_list->slot_mask) == 0) {
            //collect item
            if (s_list->completion == 1 && s_list->type != NONE_PROCESS && (q_list->complete & s_list->mask) != 0) {
                switch (add_to_storage(db, processed_item_enum_to_string(s_list->type), 1)) {
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
                        evbuffer_add_printf(returnbuffer, "could not collect due to barn size\r\n");
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
                        evbuffer_add_printf(returnbuffer, "could not collect due to silo size\r\n");
                        SET_CODE_INTERNAL_ERROR(code)
                        return returnbuffer;
                        break;
                    }
                    case (STORAGE_NOT_HANDLED): {
                        evbuffer_add_printf(returnbuffer, "could not collect due to not being implemented\r\n");
                        SET_CODE_INTERNAL_ERROR(code)
                        return returnbuffer;
                        break;
                    }
                }
                //item got collected
                s_list->type = NONE_PROCESS;
                s_list->completion = 0;
                //remove from the queue's tracking
                q_list->complete = (q_list->complete ^ s_list->mask) & q_list->slot_mask;
                q_list->fill_state = (q_list->fill_state ^ s_list->mask) & q_list->slot_mask;

                //db
                set_grain_mill_completion(db, q_list->queue_number, s_list->slot_number, 0);
                set_grain_mill_type(db, q_list->queue_number, s_list->slot_number, processed_item_enum_to_string(NONE_PROCESS));
                clear_grain_mill_time(db, q_list->queue_number, s_list->slot_number);

                update_meta(db, 2, "xp");
                xp_check(db);
            }

            //increment s_list and collect_ptr
            s_list = s_list->next;
            q_list->collect_ptr = s_list;
        }

        q_list = q_list->next;
    }

    SET_CODE_OK(code)
    return returnbuffer;
}

struct evbuffer *add_item_to_grain_mill_queue(sqlite3 *db, queue_list *queue, const char *product, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg), int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    enum processed_item item = processed_item_string_to_enum(product);

    if (item == NONE_PROCESS || get_product_type_processed_item(item) != GRAIN_MILL_PRODUCT) {
        evbuffer_add_printf(returnbuffer, "invalid product for grain mill\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    struct recipes recipe = grain_mill_recipes[item];
    const char *sanitized_string_product = processed_item_enum_to_string(item);
    const char *sanitized_string_ingredient = any_item_enum_to_string(recipe.ingredient);

    if (items_available(db, sanitized_string_ingredient, recipe.quanity) == 0) {
        evbuffer_add_printf(returnbuffer, "not enough %s to make %s\r\n", sanitized_string_ingredient, sanitized_string_product);
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //test for openings in queue
    queue_list *q_list = queue;
    int open_slots = 0;
    while (q_list != NULL) {
        if ((q_list->fill_state ^ q_list->slot_mask) != 0) {
            open_slots++;
        }
        q_list = q_list->next;
    }

    if (open_slots == 0) {
        evbuffer_add_printf(returnbuffer, "no slots open in grain mill queue\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //remove item
    switch (remove_from_storage(db, sanitized_string_ingredient, recipe.quanity)) {
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
            evbuffer_add_printf(returnbuffer, "not enough %s in barn\r\n", sanitized_string_ingredient);
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
            evbuffer_add_printf(returnbuffer, "not enough %s in silo\r\n", sanitized_string_ingredient);
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

    //add to queue
    //reset
    //simple queue filling algroithum
    q_list = queue;
    while (q_list != NULL) {
        if ((q_list->fill_state ^ q_list->slot_mask) == 0) {
            //queue is full
            q_list = q_list->next;
            continue;
        }

        slot_list *s_list_new = q_list->new_task_ptr;
        slot_list *s_list_active = q_list->active_ptr;
        time_t start_time = 0;
        if ((q_list->activity & q_list->slot_mask) == 0 || s_list_new == s_list_active) {
            //case is either all new queue or empty
            start_time = time(NULL);
        }
        else if (s_list_active->next == s_list_new) {
            start_time = get_grain_mill_end_time(db, q_list->queue_number, s_list_active->slot_number);
        }
        else {
            //increment s_list_active until s_list_active->next == s_list_new
            while (s_list_active->next != s_list_new) {
                s_list_active = s_list_active->next;
            }
            start_time = get_grain_mill_end_time(db, q_list->queue_number, s_list_active->slot_number);
        }

        if (s_list_new->type == NONE_PROCESS && (q_list->fill_state & s_list_new->mask) == 0) {
            if (s_list_new->event == NULL) {
                //make the event
                struct box_for_list_and_db *box = malloc(sizeof(struct box_for_list_and_db));
                box->db = db;
                box->list = queue;

                s_list_new->event = event_new(base, -1, 0, cb, box);

                if (s_list_new->event == NULL) {
                    evbuffer_add_printf(returnbuffer, "error making event\r\n");
                    SET_CODE_INTERNAL_ERROR(code);
                    return returnbuffer;
                }
            }

            s_list_new->type = item;
            set_grain_mill_completion(db, q_list->queue_number, s_list_new->slot_number, 0);
            set_grain_mill_type(db, q_list->queue_number, s_list_new->slot_number, sanitized_string_product);
            set_grain_mill_start_time(db, q_list->queue_number, s_list_new->slot_number, start_time);

            const struct timeval tv = grain_mill_times[item];
            set_grain_mill_end_time(db, q_list->queue_number, s_list_new->slot_number, start_time + tv.tv_sec);

            q_list->fill_state |= s_list_new->mask;

            if (q_list->activity == 0) {
                q_list->activity |= s_list_new->mask;
                int rc = event_add(s_list_new->event, &tv);
                if (rc != 0) {
                    evbuffer_add_printf(returnbuffer, "error adding event\r\n");
                    SET_CODE_INTERNAL_ERROR(code)
                    return returnbuffer;
                }
            }

            s_list_new = s_list_new->next;
            q_list->new_task_ptr = s_list_new;
            break;
        }
        q_list = q_list->next;
    }


    SET_CODE_OK(code)
    return returnbuffer;
}

void mark_grain_mill_item_as_complete(sqlite3 *db, queue_list *queue) {
    queue_list *q_list = queue;
    slot_list *s_list = q_list->active_ptr;

    s_list->completion = 1;
    set_grain_mill_completion(db, q_list->queue_number, s_list->slot_number, 1);

    q_list->complete |= s_list->mask;

    q_list->activity = q_list->activity ^ s_list->mask;

    s_list = s_list->next;

    if (s_list->event != NULL && s_list->type != NONE_PROCESS && (q_list->fill_state & s_list->mask) != 0) {
        const struct timeval tv = grain_mill_times[s_list->type];
        int rc = event_add(s_list->event, &tv);
        if (rc != 0) {
            syslog(LOG_WARNING, "error adding event grain mill queue%d slot%d", q_list->queue_number, s_list->slot_number);
            return;
        }

        q_list->activity |= s_list->mask;
    }

    q_list->active_ptr = s_list;
}
