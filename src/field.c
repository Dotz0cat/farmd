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

     if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

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

     if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

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
            enum storage storage_place = get_storage_type_field(list->type);

            if (storage_place == SILO) {

                if (get_silo_allocation(db) < (get_silo_max(db) - 1)) {
                    if (add_item_to_silo(db, field_crop_enum_to_string(list->type), UNLOCKED) != 0) {
                        //INSERT OR IGNORE shouldnt cause an issue or any slowdowns I hope
                        evbuffer_add_printf(returnbuffer, "silo error\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                    if (update_silo(db, field_crop_enum_to_string(list->type), 2) != 0) {
                        evbuffer_add_printf(returnbuffer, "silo error\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                }
                else {
                    evbuffer_add_printf(returnbuffer, "could not harvest field%d due to silo size\r\n", list->field_number);
                    *code = 500;
                    return returnbuffer;
                }
            }
            else if (storage_place == BARN) {

                if (get_barn_allocation(db) < (get_barn_max(db) - 1)) {
                    if (add_item_to_silo(db, field_crop_enum_to_string(list->type), UNLOCKED) != 0) {
                        //INSERT OR IGNORE shouldnt cause an issue or any slowdowns I hope
                        evbuffer_add_printf(returnbuffer, "barn error\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                    if (update_barn(db, field_crop_enum_to_string(list->type), 2) != 0) {
                        evbuffer_add_printf(returnbuffer, "barn error\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                }
                else {
                    evbuffer_add_printf(returnbuffer, "could not harvest field%d due to barn size\r\n", list->field_number);
                    *code = 500;
                    return returnbuffer;
                }
            }
            else {
                evbuffer_add_printf(returnbuffer, "could not harvest field%d due to not being implemented\r\n", list->field_number);
                *code = 500;
                return returnbuffer;
            }

            list->type = NONE_FIELD;
            list->completion = 0;
            remove_field(db, list->field_number);
            //reset the complete flag to false
            set_field_completion(db, list->field_number, 0);
            //clear the field time as it had now been harvested
            clear_field_time(db, list->field_number);
            update_meta(db, 2, "xp");
            //xp_check(db);
        }

        list = list->next;
    } while (list != NULL);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *plant_field(sqlite3 *db, fields_list **field_list, const char *crop, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *user_data), int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (get_number_of_fields(db) == 0) {
        evbuffer_add_printf(returnbuffer, "no fields\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (field_list == NULL) {
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

            enum storage store = get_storage_type_field(type);

            //consume crops or cash
            if (store == SILO) {
                if (silo_query_db(db, sanitized_string) > 0) {
                    if (update_silo(db, sanitized_string, -1) != 0) {
                        evbuffer_add_printf(returnbuffer, "could not update silo\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                }
            }
            else if (store == BARN) {
                if (barn_query_db(db, sanitized_string) > 0) {
                    if (update_barn(db, sanitized_string, -1) != 0) {
                        evbuffer_add_printf(returnbuffer, "could not update barn\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                }
            }
            else if (get_money(db) > price) {
                if (update_meta(db, (-1 * price), "Money") != 0) {
                    evbuffer_add_printf(returnbuffer, "could not update money\r\n");
                    *code = 500;
                    return returnbuffer;
                }
            }
            else {
                evbuffer_add_printf(returnbuffer, "could not plant or buy\r\n");
                *code = 500;
                return returnbuffer;
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

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    //check skill tree first
    int current = get_number_of_fields(db);
    int skill_level = get_skill_status(db, "Fields");
    if (current < (skill_level * 3)) {
        //price is 2^current fields for next
        //was pow(2, current)
        int price = 2 << current;
        if (get_money(db) > price) {
            if (update_meta(db, (-1 * price), "Money") == 0) {
                if (update_meta(db, 1, "Fields") != 0) {
                    evbuffer_add_printf(returnbuffer, "error adding field\r\n");
                    *code = 500;
                    return returnbuffer;
                }
            }
            else {
                evbuffer_add_printf(returnbuffer, "error subtracting money\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
        else {
            evbuffer_add_printf(returnbuffer, "not enough money to buy field\r\n");
            *code = 500;
            return returnbuffer;
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
