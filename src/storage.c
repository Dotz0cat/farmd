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

#include "storage.h"

enum storage_errors add_to_storage(sqlite3 *db, const char *item, const int number_of_items) {
    enum storage storage_place = get_storage_type_string(item);
    enum item_type type = get_product_type_string(item);

    if (storage_place == BARN) {
        //check allocation
        if ((get_barn_max(db) - get_barn_allocation(db)) >= number_of_items) {
            //check if item is there
            if (barn_query_db(db, item) != -1) {
                //add item
                if (update_barn(db, item, number_of_items) != 0) {
                    return BARN_UPDATE;
                }
            }
            else {
                if (type != SPECIAL_PRODUCT) {
                    if (get_skill_status(db, item) == 0) {
                        if (add_item_to_barn(db, item, LOCKED) != 0) {
                            return BARN_ADD;
                        }
                        if (update_barn(db, item, number_of_items) != 0) {
                            return BARN_UPDATE;
                        }
                    }
                    else {
                        if (add_item_to_barn(db, item, UNLOCKED) != 0) {
                            return BARN_ADD;
                        }
                        if (update_barn(db, item, number_of_items) != 0) {
                            return BARN_UPDATE;
                        }
                    }
                }
                else {
                    //if it is a special product it is special
                    if (add_item_to_barn(db, item, SPECIAL) != 0) {
                        return BARN_ADD;
                    }
                    if (update_barn(db, item, number_of_items) != 0) {
                        return BARN_UPDATE;
                    }
                }
            }
        }
        else {
            return BARN_SIZE;
        }
    }
    else if (storage_place == SILO) {
        //check allocation
        if ((get_silo_max(db) - get_silo_allocation(db)) >= number_of_items) {
            //check if item is there
            if (silo_query_db(db, item) != -1) {
                //add item
                if (update_silo(db, item, number_of_items) != 0) {
                    return SILO_UPDATE;
                }
            }
            else {
                if (type != SPECIAL_PRODUCT) {
                    if (get_skill_status(db, item) == 0) {
                        if (add_item_to_silo(db, item, LOCKED) != 0) {
                            return SILO_ADD;
                        }
                        if (update_silo(db, item, number_of_items) != 0) {
                            return SILO_UPDATE;
                        }
                    }
                    else {
                        if (add_item_to_silo(db, item, UNLOCKED) != 0) {
                            return SILO_ADD;
                        }
                        if (update_silo(db, item, number_of_items) != 0) {
                            return SILO_UPDATE;
                        }
                    }
                }
                else {
                    //if it is a special product it is special
                    if (add_item_to_silo(db, item, SPECIAL) != 0) {
                        return SILO_ADD;
                    }
                    if (update_silo(db, item, number_of_items) != 0) {
                        return SILO_UPDATE;
                    }
                }
            }
        }
        else {
            return SILO_SIZE;
        }
    }
    else {
        return STORAGE_NOT_HANDLED;
    }
    return NO_STORAGE_ERROR;
}

enum storage_errors remove_from_storage(sqlite3 *db, const char *item, const int number_of_items) {
    enum storage storage_place = get_storage_type_string(item);

    if (storage_place == BARN) {
        //check if item is there
        if (barn_query_db(db, item) > 0) {
            //remove item
            if (update_barn(db, item, (-1 * number_of_items)) != 0) {
                return BARN_UPDATE;
            }
        }
        else {
            return BARN_SIZE;
        }
    }
    else if (storage_place == SILO) {
        //check if item is there
        if (silo_query_db(db, item) > 0) {
            //remove item
            if (update_silo(db, item, (-1 * number_of_items)) != 0) {
                return SILO_UPDATE;
            }
        }
        else {
            return SILO_SIZE;
        }
    }
    else {
        return STORAGE_NOT_HANDLED;
    }

    return NO_STORAGE_ERROR;
}

int items_in_storage(sqlite3 *db, const char *item) {
    enum storage storage_place = get_storage_type_string(item);

    if (storage_place == BARN) {
        return barn_query_db(db, item);
    }
    else if (storage_place == SILO) {
        return silo_query_db(db, item);
    }
    else {
        return -1;
    }
}

int unlock_item_status(sqlite3 *db, const char *item) {
    enum storage store = get_storage_type_string(item);
    if (store == SILO) {
        if (check_silo_item_status(db, item) == LOCKED) {
            if (update_silo_status(db, item, UNLOCKED) != 0) {
                return -1;
            }
        }
    }
    else if (store == BARN) {
        if (check_barn_item_status(db, item) == LOCKED) {
            if (update_barn_status(db, item, UNLOCKED) != 0) {
                return -1;
            }
        }
    }
    else {
        return 0;
    }

    return 0;
}
