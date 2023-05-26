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

#ifndef INITAL_VALUES_H
#define INITAL_VALUES_H

#include <stdlib.h>
#include <stdint.h>
#include "save.h"

int add_inital_save_values(sqlite3* db);
const char* skill_dep_check(sqlite3* db, const char* skill);
static inline uint64_t hash(const char* key);

#endif /* INITAL_VALUES_H */
