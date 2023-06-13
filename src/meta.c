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

#include "meta.h"

void xp_check(sqlite3 *db) {
    int level = get_level(db);
    level--;
    //was pow(2, level) * 10;
    int xp_needed = 10 << level;
    //xp needed for level 2 is 10
    //level 3 is 20
    //level 4 is 40
    //level 5 is 80
    //level 6 is 160
    int xp = get_xp(db);
    if (xp >= xp_needed) {
        if (level_up(db, xp_needed) != 0) {
            syslog(LOG_WARNING, "error leveling up");
            return;
        }
        if (update_meta(db, 2, "SkillPoints") != 0) {
            syslog(LOG_WARNING, "error adding skill points");
            return;
        }
    }
}

struct evbuffer *view_money(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    int money = get_money(db);
    evbuffer_add_printf(returnbuffer, "money: %d\r\n", money);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *view_level(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    int level = get_level(db);
    evbuffer_add_printf(returnbuffer, "level: %d\r\n", level);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *view_xp(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    int xp = get_xp(db);
    evbuffer_add_printf(returnbuffer, "xp: %d\r\n", xp);

    *code = 200;
    return returnbuffer;
}

struct evbuffer *view_skill_points(sqlite3 *db, int *code) {
    struct evbuffer *returnbuffer = evbuffer_new();

    if (db == NULL) {
        evbuffer_add_printf(returnbuffer, "no save open\r\n");
        *code = 500;
        return returnbuffer;
    }

    int skill_points = get_skill_points(db);
    evbuffer_add_printf(returnbuffer, "Skill points: %d\r\n", skill_points);

    *code = 200;
    return returnbuffer;
}