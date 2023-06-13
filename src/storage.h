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

#ifndef STORAGE_H
#define STORAGE_H

#include "list.h"
#include "save.h"

enum storage_errors {
    NO_STORAGE_ERROR,
    BARN_UPDATE,
    BARN_ADD,
    BARN_SIZE,
    SILO_UPDATE,
    SILO_ADD,
    SILO_SIZE,
    STORAGE_NOT_HANDLED
};

enum storage_errors add_to_storage(sqlite3 *db, const char *item, const int number_of_items);
enum storage_errors remove_from_storage(sqlite3 *db, const char *item, const int number_of_items);
int items_in_storage(sqlite3 *db, const char *item);
int unlock_item_status(sqlite3 *db, const char *item);

#endif /* STORAGE_H */