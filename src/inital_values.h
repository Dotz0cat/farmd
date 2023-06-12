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

#ifndef INITAL_VALUES_H
#define INITAL_VALUES_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "save.h"

//skill_name, dependency
#define SKILL_DEP_TABLE \
X("Farming", NULL), \
X("Fields", "Farming"), \
X("wheat", "Farming"), \
X("corn", "Farming"), \
X("potatos", "Farming"), \
X("beets", "Farming"), \
X("sugarcane", "Farming"), \
X("turnips", "Farming"), \
X("tomatos", "Farming"), \
X("cucumbers", "Farming"), \
X("okra", "Farming"), \
X("TreePlots", "Farming"), \
X("pears", "Farming"), \
X("apples", "Farming"), \
X("oranges", "Farming"), \
X("peaches", "Farming"), \
X("LiveStock", NULL), \
X("Processing", NULL), \
X("Dairy", "Processing"), \
X("pasteurized", "Dairy"), \
X("butter", "Dairy"), \
X("cheese", "Dairy"), \
X("FeedMill", "Processing"), \
X("cowFeed", "FeedMill"), \
X("chickenFeed", "FeedMill"), \
X("Milling", "Processing"), \
X("GrainMill", "Milling"), \
X("wheatFlour", "GrainMill"), \
X("cornMeal", "GrainMill"), \
X("SugarMill", "Milling"), \
X("beetSugar", "SugarMill"), \
X("caneSugar", "SugarMill")

struct dependency_map {
    const char *skill;
    const char *dependency;
};

int add_inital_save_values(sqlite3 *db);
const char *skill_dep_check(sqlite3 *db, const char *skill);
const char *skill_sanitize(const char *skill);

#endif /* INITAL_VALUES_H */
