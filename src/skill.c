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

#include "skill.h"

#define X(a, b) {a, b}
const struct dependency_map dep_map[] = {
    SKILL_DEP_TABLE
};
#undef X

struct evbuffer *buy_skill(sqlite3 *db, const char *skill_name, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)

    const char *sanitized_string;

    if ((sanitized_string = skill_sanitize(skill_name)) == NULL) {
        evbuffer_add_printf(returnbuffer, "failed to buy %s: not valid\r\n", skill_name);
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    if (strcasecmp(sanitized_string, "Fields") == 0) {
        //limit is 1 per level
        if (get_skill_status(db, "Fields") >= get_level(db)) {
            evbuffer_add_printf(returnbuffer, "failed to buy %s: limit is 1 per level\r\n", sanitized_string);
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
        }
    }
    else if (strcasecmp(sanitized_string, "TreePlots") == 0) {
        //limit is 1 per level
        if (get_skill_status(db, "TreePlots") >= get_level(db)) {
            evbuffer_add_printf(returnbuffer, "failed to buy %s: limit is 1 per level\r\n", sanitized_string);
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
        }
    }
    else if (get_skill_status(db, sanitized_string) > 0) {
        evbuffer_add_printf(returnbuffer, "already own skill: %s\r\n", sanitized_string);
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //depenacny checking
    const char *reason = skill_dep_check(db, sanitized_string);
    if (reason != NULL) {
        evbuffer_add_printf(returnbuffer, "dependency needed: %s\r\n", reason);
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    int skill_points = get_skill_points(db);
    if (skill_points < 1) {
        evbuffer_add_printf(returnbuffer, "not enough skill points\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }
    if (update_meta(db, -1, "SkillPoints") == 0) {
        if (update_skill_tree(db, sanitized_string) != 0) {
            evbuffer_add_printf(returnbuffer, "error adding skill\r\n");
            SET_CODE_INTERNAL_ERROR(code)
            return returnbuffer;
        }
    }
    else {
        evbuffer_add_printf(returnbuffer, "error subtracting skill points\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    //unlock the item in storage
    if (unlock_item_status(db, sanitized_string) != 0) {
        evbuffer_add_printf(returnbuffer, "error unlocking item\r\n");
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    SET_CODE_OK(code)
    evbuffer_add_printf(returnbuffer, "sucessfully bought skill: %s\r\n", sanitized_string);

    return returnbuffer;
}

const char *skill_dep_check(sqlite3 *db, const char *skill) {
    for (int i = 0; i < (int) (sizeof(dep_map) / sizeof(dep_map[0])); i++) {
        if (skill == dep_map[i].skill) {
            if (dep_map[i].dependency != NULL) {
                if (get_skill_status(db, dep_map[i].dependency) != 0) {
                    return NULL;
                }
                else {
                    return dep_map[i].dependency;
                }
            }
            else {
                return NULL;
            }
        }
    }
    return "Not Found";
}

const char *skill_sanitize(const char *skill) {
    for (int i = 0; i < (int) (sizeof(dep_map) / sizeof(dep_map[0])); i++) {
        if (strcasecmp(skill, dep_map[i].skill) == 0) {
            return dep_map[i].skill;
        }
    }

    return NULL;
}

struct evbuffer *skill_status(sqlite3 *db, const char *skill, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    CHECK_SAVE_OPEN(db, returnbuffer, code)
    
    const char *sanitized_string = skill_sanitize(skill);

    if (sanitized_string == NULL) {
        evbuffer_add_printf(returnbuffer, "Skill %s: is not valid\r\n", skill);
        SET_CODE_INTERNAL_ERROR(code)
        return returnbuffer;
    }

    int skill_status = get_skill_status(db, sanitized_string);

    evbuffer_add_printf(returnbuffer, "Skill status: %s %d\r\n", sanitized_string, skill_status);

    SET_CODE_OK(code)
    return returnbuffer;
}
