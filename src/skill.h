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

#ifndef SKILL_H
#define SKILL_H

#include <string.h>
#include <event2/buffer.h>

#include "save.h"
#include "list.h"
#include "storage.h"

//skill_name, dependency
#define SKILL_DEP_TABLE \
X("Farming", NULL), \
X("Fields", "Farming"), \
X("wheat", "Fields"), \
X("corn", "Fields"), \
X("potatos", "Fields"), \
X("beets", "Fields"), \
X("sugarcane", "Fields"), \
X("turnips", "Fields"), \
X("tomatos", "Fields"), \
X("cucumbers", "Fields"), \
X("okra", "Fields"), \
X("TreePlots", "Farming"), \
X("pears", "TreePlots"), \
X("apples", "TreePlots"), \
X("oranges", "TreePlots"), \
X("peaches", "TreePlots"), \
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

struct evbuffer *buy_skill(sqlite3 *db, const char *skill_name, int *code);
const char *skill_dep_check(sqlite3 *db, const char *skill);
const char *skill_sanitize(const char *skill);
struct evbuffer *skill_status(sqlite3 *db, const char *skill, int *code);

#endif /* SKILL_H */
