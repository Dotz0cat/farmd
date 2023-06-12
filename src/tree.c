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

#include "tree.h"

//enum, string, time, buy, sell, storage, item_type, maturity time
#define X(a, b, c, d, e, f, g, h) [a]={c, 0}
static const struct timeval tree_time[] = {
    TREE_CROP_TABLE
};
#undef X

//enum, string, time, buy, sell, storage, item_type, maturity time
#define X(a, b, c, d, e, f, g, h) [a]={h, 0}
static const struct timeval tree_maturity_time[] = {
    TREE_CROP_TABLE
};
#undef X

struct evbuffer *buy_tree_plot(sqlite3 *db, trees_list **tree_list, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();
    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    int current = get_number_of_tree_plots(db);
    int skill_level = get_skill_status(db, "TreePlots");
    if (current < skill_level) {
        //price is 2^current + 1 plots for next
        //was pow(2, current + 1)
        int price = 2 << (current + 1);
        if (get_money(db) > price) {
            if (update_meta(db, (-1 * price), "Money") == 0) {
                if (update_meta(db, 1, "TreePlots") != 0) {
                    evbuffer_add_printf(returnbuffer, "error adding tree plot\r\n");
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
            evbuffer_add_printf(returnbuffer, "not enough money to buy tree plot\r\n");
            *code = 500;
            return returnbuffer;
        }
    }
    else {
        evbuffer_add_printf(returnbuffer, "not high enough skill level to buy this\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (current == 0) {
        *tree_list = make_trees_list(get_number_of_tree_plots(db));
        for (int i = 0; i < get_number_of_tree_plots(db); i++) {
            if (add_tree(db, i) != 0) {
                evbuffer_add_printf(returnbuffer, "error adding tree plot\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
    }
    else {
        if (amend_trees_list(*tree_list, get_number_of_tree_plots(db)) != 0) {
            evbuffer_add_printf(returnbuffer, "error amending tree list\r\n");
            *code = 500;
            return returnbuffer;
        }
        for (int i = current; i < get_number_of_tree_plots(db); i++) {
            if (add_tree(db, i) != 0) {
                evbuffer_add_printf(returnbuffer, "error adding tree plot\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
    }

    evbuffer_add_printf(returnbuffer, "sucessfully bought tree plot\r\n");
    *code = 200;
    return returnbuffer;
}

struct evbuffer *plant_tree(sqlite3 *db, trees_list **tree_list, const char *crop, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *user_data), int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (get_number_of_tree_plots(db) == 0) {
        evbuffer_add_printf(returnbuffer, "no tree plots\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (*tree_list == NULL) {
        *tree_list = make_trees_list(get_number_of_tree_plots(db));
        if (*tree_list == NULL) {
            evbuffer_add_printf(returnbuffer, "could not make trees\r\n");
            *code = 500;
            return returnbuffer;
        }
        for (int i = 0; i < get_number_of_tree_plots(db); i++) {
            if (add_tree(db, i) != 0) {
                evbuffer_add_printf(returnbuffer, "error adding tree to db\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
    }

    int current;
    if ((current = get_number_of_trees_list(*tree_list)) < get_number_of_tree_plots(db)) {
        if (amend_trees_list(*tree_list, get_number_of_tree_plots(db)) != 0) {
            evbuffer_add_printf(returnbuffer, "could not amend trees\r\n");
            *code = 500;
            return returnbuffer;
        }
        for (int i = current; i < get_number_of_tree_plots(db); i++) {
            if (add_tree(db, i) != 0) {
                evbuffer_add_printf(returnbuffer, "error adding tree to db\r\n");
                *code = 500;
                return returnbuffer;
            }
        }
    }

    enum tree_crop type;

    type = tree_crop_string_to_enum(crop);

    if (type == NONE_TREE) {
        evbuffer_add_printf(returnbuffer, "%s is not a correct query\r\n", crop);
        *code = 500;
        return returnbuffer;
    }

    const char *sanitized_string = tree_crop_enum_to_string(type);

    if (get_skill_status(db, sanitized_string) == 0) {
        evbuffer_add_printf(returnbuffer, "currently do not own %s skill\r\n", sanitized_string);
        *code = 500;
        return returnbuffer;
    }

    //Plant tree plot
    trees_list *list = *tree_list;

    int tree_plot = 0;

    //so only one tree can be planted at a time
    int done = 0;

    do {
        if (list->type == NONE_TREE) {
            //plant a tree
            //check if event is clear
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

            int price = tree_crop_buy_cost(type);

            enum storage storage_place = get_storage_type_tree(type);

            //consume crops or cash
            if (storage_place == BARN) {
                if (barn_query_db(db, sanitized_string) > 0) {
                    if (update_barn(db, sanitized_string, -1) != 0) {
                        evbuffer_add_printf(returnbuffer, "could not update barn\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                }
            }
            else if (storage_place == SILO) {
                if (silo_query_db(db, sanitized_string) > 0) {
                    if (update_silo(db, sanitized_string, -1) != 0) {
                        evbuffer_add_printf(returnbuffer, "could not update silo\r\n");
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
            //add to trees table
            set_tree_type(db, list->tree_number, sanitized_string);
            set_tree_completion(db, list->tree_number, 0);
            set_tree_maturity(db, list->tree_number, 0);

            //set the completetion time
            set_tree_time(db, list->tree_number, tree_maturity_time[type].tv_sec);

            //set the tree maturity timer
            const struct timeval *tv = &tree_maturity_time[type];
            int rc = event_add(list->event, tv);
            if (rc != 0) {
                evbuffer_add_printf(returnbuffer, "error adding event\r\n");
                *code = 500;
                return returnbuffer;
            }

            tree_plot = list->tree_number;

            done = 1;
        }

       list = list->next;

    } while(list != NULL && done == 0);

    if (done == 0) {
        evbuffer_add_printf(returnbuffer, "no open tree plot\r\n");
        *code = 500;
        return returnbuffer;
    }

    evbuffer_add_printf(returnbuffer, "planted tree plot: %d\r\n", tree_plot);
    *code = 200;
    return returnbuffer;
}

struct evbuffer *harvest_tree(sqlite3 *db, trees_list *tree_list, struct event_base *base, void (*cb)(evutil_socket_t fd, short events, void *user_data), int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (get_number_of_tree_plots(db) == 0) {
        evbuffer_add_printf(returnbuffer, "no trees\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (tree_list == NULL) {
        evbuffer_add_printf(returnbuffer, "no trees\r\n");
        *code = 500;
        return returnbuffer;
    }

    trees_list *list = tree_list;

    do {
        if (list->completion == 1 && list->type != NONE_TREE) {
            enum storage storage_place = get_storage_type_tree(list->type);

            if (storage_place == BARN) {
                if (get_barn_allocation(db) < (get_barn_max(db)) - 1) {
                    if (add_item_to_barn(db, tree_crop_enum_to_string(list->type), UNLOCKED) != 0) {
                        //INSERT OR IGNORE shouldnt cause an issue or any slowdowns I hope
                        evbuffer_add_printf(returnbuffer, "barn error\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                    if (update_barn(db, tree_crop_enum_to_string(list->type), 2) != 0) {
                        evbuffer_add_printf(returnbuffer, "barn error\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                }
                else {
                    evbuffer_add_printf(returnbuffer, "could not harvest tree%d due to barn size\r\n", list->tree_number);
                    *code = 500;
                    return returnbuffer;
                }
            }
            else if (storage_place == SILO) {
                if (get_silo_allocation(db) < (get_silo_max(db)) - 1) {
                    if (add_item_to_silo(db, tree_crop_enum_to_string(list->type), UNLOCKED) != 0) {
                        //INSERT OR IGNORE shouldnt cause an issue or any slowdowns I hope
                        evbuffer_add_printf(returnbuffer, "silo error\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                    if (update_silo(db, tree_crop_enum_to_string(list->type), 2) != 0) {
                        evbuffer_add_printf(returnbuffer, "silo error\r\n");
                        *code = 500;
                        return returnbuffer;
                    }
                }
                else {
                    evbuffer_add_printf(returnbuffer, "could not harvest tree%d due to silo size\r\n", list->tree_number);
                    *code = 500;
                    return returnbuffer;
                }
            }
            else {
                evbuffer_add_printf(returnbuffer, "could not harvest tree%d due to not being implemented\r\n", list->tree_number);
                *code = 500;
                return returnbuffer;
            }
            
            list->completion = 0;

            //update the db
            set_tree_completion(db, list->tree_number, 0);
            set_tree_time(db, list->tree_number, tree_time[list->type].tv_sec);

            const struct timeval *tv = &tree_time[list->type];
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
            int rc = event_add(list->event, tv);
            if (rc != 0) {
                evbuffer_add_printf(returnbuffer, "error adding event\r\n");
                *code = 500;
                return returnbuffer;
            }

            update_meta(db, 2, "xp");
            //xp_check(db);
        }
        list = list->next;
    } while (list != NULL);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *tree_status(sqlite3 *db, trees_list *tree_list, int *code) {
    struct evbuffer * returnbuffer = evbuffer_new();

        if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (get_number_of_tree_plots(db) == 0) {
        evbuffer_add_printf(returnbuffer, "no tree plots\r\n");
        *code = 500;
        return returnbuffer;
    }

    if (tree_list == NULL) {
        evbuffer_add_printf(returnbuffer, "no trees\r\n");
        *code = 500;
        return returnbuffer;
    }

    trees_list *list = tree_list;

    //check for races
    do {
        char *complete;
        if (list->completion == 1) {
            complete = "ready";
            set_tree_completion(db, list->tree_number, 1);
        }
        else if (get_tree_completion(db, list->tree_number) == 1) {
            complete = "ready";
        }
        else {
            complete = "not ready";
        }

        char *maturity;
        if (list->maturity == 1) {
            maturity = "mature";
        }
        else if (get_tree_maturity(db, list->tree_number) == 1) {
            maturity = "mature";
        }
        else {
            maturity = "not mature";
        }
        evbuffer_add_printf(returnbuffer, "tree%d: %s %s %s\r\n", list->tree_number, tree_crop_enum_to_string(list->type), maturity, complete);
        list = list->next;
    } while (list != NULL);

    *code = 200;
    return returnbuffer;
}
