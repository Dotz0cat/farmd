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

#include "field.h"

//enum, string, time, buy, sell, storage, item_type
#define X(a, b, c, d, e, f, g) [a]={c, 0}
static const struct timeval field_time[] = {
    FIELD_CROP_TABLE
};
#undef X

struct evbuffer *field_status(sqlite3 *db, fields_list *field_list, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    if (get_number_of_fields(db) == 0) {
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (field_list == NULL) {
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        *code = 500;
        return returnbuffer;
    }

    fields_list *list = field_list;

    //check for races
    do {
        char *complete;
        if (list->completion == 1) {
            complete = "ready";
            //make sure its right
            set_field_completion(db, list->field_number, 1);
        }
        else if (get_field_completion(db, list->field_number) == 1) {
            complete = "ready";
        }
        else {
            complete = "not ready";
        }
        evbuffer_add_printf(returnbuffer, "field%d: %s %s\r\n", list->field_number, field_crop_enum_to_string(list->type), complete);
        list = list->next;
    } while (list != NULL);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *harvest_field(sqlite3 *db, fields_list *field_list, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    if (get_number_of_fields(db) == 0) {
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (field_list == NULL) {
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        *code = 500;
        return returnbuffer;
    }

    fields_list *list = field_list;

    do {
        if (list->completion == 1 && list->type != NONE_FIELD) {
            switch (add_to_storage(db, field_crop_enum_to_string(list->type), 2)) {
                case (NO_STORAGE_ERROR): {
                    break;
                }
                case (BARN_UPDATE): {
                    evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                    *code = 500;
                    return returnbuffer;
                    break;
                }
                case (BARN_ADD): {
                    evbuffer_add_printf(returnbuffer, "error adding item to barn\r\n");
                    *code = 500;
                    return returnbuffer;
                    break;
                }
                case (BARN_SIZE): {
                    evbuffer_add_printf(returnbuffer, "could not harvest field%d due to barn size\r\n", list->field_number);
                    *code = 500;
                    return returnbuffer;
                    break;
                }
                case (SILO_UPDATE): {
                    evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
                    *code = 500;
                    return returnbuffer;
                    break;
                }
                case (SILO_ADD): {
                    evbuffer_add_printf(returnbuffer, "error adding item to silo\r\n");
                    *code = 500;
                    return returnbuffer;
                    break;
                }
                case (SILO_SIZE): {
                    evbuffer_add_printf(returnbuffer, "could not harvest field%d due to silo size\r\n", list->field_number);
                    *code = 500;
                    return returnbuffer;
                    break;
                }
                case (STORAGE_NOT_HANDLED): {
                    evbuffer_add_printf(returnbuffer, "could not harvest field%d due to not being implemented\r\n", list->field_number);
                    *code = 500;
                    return returnbuffer;
                    break;
                }
            }

            list->type = NONE_FIELD;
            list->completion = 0;
            remove_field(db, list->field_number);
            //reset the complete flag to false
            set_field_completion(db, list->field_number, 0);
            //clear the field time as it had now been harvested
            clear_field_time(db, list->field_number);
            update_meta(db, 2, "xp");
            xp_check(db);
        }

        list = list->next;
    } while (list != NULL);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *plant_field(sqlite3 *db, fields_list **field_list, const char *crop, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *user_data), int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    if (get_number_of_fields(db) == 0) {
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (*field_list == NULL) {
        *field_list = make_fields_list(get_number_of_fields(db));
        if (*field_list == NULL) {
            evbuffer_add_printf(returnbuffer, "could not make fields\r\n");
            *code = 500;
            return returnbuffer;
        }
         for (int i = 0; i < get_number_of_fields(db); i++) {
            if (add_field(db, i) != 0) {
                evbuffer_add_printf(returnbuffer, "error adding field to db\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
    }

    int current = 0;
    if ((current = get_number_of_fields_list(*field_list)) < get_number_of_fields(db)) {
        if (amend_fields_list(*field_list, get_number_of_fields(db)) != 0) {
            evbuffer_add_printf(returnbuffer, "could not amend fields\r\n");
            *code = 500;
            return returnbuffer;
        }
        for (int i = current; i < get_number_of_fields(db); i++) {
            if (add_field(db, i) != 0) {
                evbuffer_add_printf(returnbuffer, "error adding field to db\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
    }

    enum field_crop type;

    type = field_crop_string_to_enum(crop);
    if (type == NONE_FIELD) {
        evbuffer_add_printf(returnbuffer, "%s is not a correct query\r\n", crop);
        *code = 500;
        return returnbuffer;
    }

    const char *sanitized_string = field_crop_enum_to_string(type);

    if (get_skill_status(db, sanitized_string) == 0) {
        evbuffer_add_printf(returnbuffer, "currently do not own %s skill\r\n", sanitized_string);
        *code = 500;
        return returnbuffer;
    }

    fields_list *list = *field_list;

    int planted = 0;

    do {
        if (list->type == NONE_FIELD) {
            if (list->event == NULL) {
                struct box_for_list_and_db *box = malloc(sizeof(struct box_for_list_and_db));
                box->list = list;
                box->db = db;

                list->event = event_new(base, -1, 0, cb, box);
                if (list->event == NULL) {
                    evbuffer_add_printf(returnbuffer, "error making event\r\n");
                    *code = 500;
                    return returnbuffer;
                }
            }

            int price = field_crop_buy_cost(type);

            //consume crops or cash
            switch(consume_crops_or_cash_price_hint(db, sanitized_string, price)) {
                case (NO_STORAGE_ERROR): {
                    break;
                }
                case (CONSUME_OR_BUY_BARN): {
                    evbuffer_add_printf(returnbuffer, "error updating barn\r\n");
                    *code = 500;
                    return returnbuffer;
                    break;
                }
                case (CONSUME_OR_BUY_SILO): {
                    evbuffer_add_printf(returnbuffer, "error updating silo\r\n");
                    *code = 500;
                    return returnbuffer;
                    break;
                }
                case (CONSUME_OR_BUY_NOT_ENOUGH_MONEY): {
                    evbuffer_add_printf(returnbuffer, "not enough money\r\n");
                    *code = 500;
                    return returnbuffer;
                    break;
                }
                case (CONSUME_OR_BUY_MONEY_ERROR): {
                    evbuffer_add_printf(returnbuffer, "error subtracting money\r\n");
                    *code = 500;
                    return returnbuffer;
                }
                case (COULD_NOT_CONSUME_OR_BUY): {
                    evbuffer_add_printf(returnbuffer, "could not plant or buy\r\n");
                    *code = 500;
                    return returnbuffer;
                    break;
                }
            }

            list->type = type;
            set_field_type(db, list->field_number, field_crop_enum_to_string(type));
            //set field completion
            set_field_completion(db, list->field_number, 0);
            //set time it should complete
            set_field_time(db, list->field_number, field_time[type].tv_sec);

            const struct timeval *tv = &field_time[type];
            int rc = event_add(list->event, tv);
            if (rc != 0) {
                evbuffer_add_printf(returnbuffer, "error adding event\r\n");
                *code = 500;
                return returnbuffer;
            }
            planted++;
        } 
        list = list->next;
    } while (list != NULL);

    evbuffer_add_printf(returnbuffer, "planted: %d\r\n", planted);
    *code = 200;

    return returnbuffer;
}

struct evbuffer *buy_field(sqlite3 *db, fields_list **field_list, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    //check skill tree first
    int current = get_number_of_fields(db);
    int skill_level = get_skill_status(db, "Fields");
    if (current < (skill_level * 3)) {
        //price is 2^current fields for next
        //was pow(2, current)
        int price = 2 << current;
        switch (subtract_money(db, price)) {
            case (NO_MONEY_ERROR): {
                if (update_meta(db, 1, "Fields") != 0) {
                    evbuffer_add_printf(returnbuffer, "error adding field\r\n");
                    *code = 500;
                    return returnbuffer;
                }
                break;
            }
            case (NOT_ENOUGH): {
                evbuffer_add_printf(returnbuffer, "not enough money to buy field\r\n");
                *code = 500;
                return returnbuffer;
                break;
            }
            case (ERROR_UPDATING): {
                evbuffer_add_printf(returnbuffer, "error subtracting money\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
    }
    else {
        evbuffer_add_printf(returnbuffer, "not high enough skill level to buy field\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (current == 0) {
        *field_list = make_fields_list(get_number_of_fields(db));

        //get the sql in shape
        int fields = get_number_of_fields(db);
        fields_list *list = *field_list;
        for (int i = 0; i < fields; i++) {
            if (add_field(db, list->field_number) != 0) {
                evbuffer_add_printf(returnbuffer, "error adding field to db\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
    }
    else {
        if (amend_fields_list(*field_list, get_number_of_fields(db)) != 0) {
            evbuffer_add_printf(returnbuffer, "error amending field list\r\n");
            *code = 500;
            return returnbuffer;
        }
        for (int i = current; i < get_number_of_fields(db); i++) {
            if (add_field(db, i) != 0) {
                evbuffer_add_printf(returnbuffer, "error adding field to db\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
    }

    evbuffer_add_printf(returnbuffer, "sucessfully bought field\r\n");
    *code = 200;
    return returnbuffer;
}

void populate_fields(sqlite3 *db, fields_list **field_list, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg)) {

    //populate fields
    int fields = get_number_of_fields(db);

    *field_list = make_fields_list(fields);

    if (fields <= 0) {
        return;
    }

    fields_list *list = *field_list;
    for (int i = 0; i < fields; i++) {
        char *field_type = get_field_type(db, i);
        list->type = field_crop_string_to_enum(field_type);
        free(field_type);

        if (list->type != NONE_FIELD) {
            setup_field_completion(db, list, base, cb);
        }
        list = list->next;
    }
}

void setup_field_completion(sqlite3 *db, fields_list *list, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *arg)) {
    if (get_field_completion(db, list->field_number) == 0) {
        time_t now = time(NULL);
        time_t time_from_db = get_field_time(db, list->field_number);

        if (time_from_db > now) {
            //make an event as it has not finshed
            struct timeval tv;
            tv.tv_sec = time_from_db - now;
            tv.tv_usec = 0;

            struct box_for_list_and_db *box = malloc(sizeof(struct box_for_list_and_db));
            box->list = list;
            box->db = db;
            list->event = event_new(base, -1, 0, cb, box);

            event_add(list->event, &tv);
        }
        else {
            //if it is not less then it is equal or greater
            list->completion = 1;
            set_field_completion(db, list->field_number, 1);
        }
    }
    else {
        list->completion = 1;
    }
}

void free_fields(fields_list **list) {
    if (*list != NULL) {
        fields_list *field_list = *list;
        while (field_list != NULL) {
            if (field_list->event != NULL) {
                event_del(field_list->event);
                void *temp;
                if ((temp = event_get_callback_arg(field_list->event)) != NULL) {
                    free(temp);
                }
                event_free(field_list->event);
            }
            fields_list *temp = field_list;
            field_list = field_list->next;
            free(temp);
        }
    }
    *list = NULL;
}

void ping_fields(sqlite3 *db) {
    int fields = get_number_of_fields(db);
    if (fields <= 0) {
        return;
    }
    for (int i = 0; i < fields; i++) {
        char *field_type = get_field_type(db, i);
        enum field_crop type = field_crop_string_to_enum(field_type);
        free(field_type);

        if (type != NONE_FIELD) {

            if (get_field_completion(db, i) == 0) {
                time_t now = time(NULL);
                time_t time_from_db = get_field_time(db, i);

                if (time_from_db <= now) {
                    set_field_completion(db, i, 1);
                }
            }
        }
    }
}
