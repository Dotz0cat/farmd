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

#include "save.h"

int create_save(const char* filename) {
    sqlite3* db;

    int rc = sqlite3_open(filename, &db);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));

        sqlite3_close(db);

        return 1;
    }

    char* sql = "CREATE TABLE Barn (Item TEXT UNIQUE, Quantity INT CHECK(Quantity >= 0), Status TEXT CHECK(Status IN ('UNLOCKED', 'LOCKED', 'SPECIAL')));"
                "CREATE TABLE Silo (Item TEXT UNIQUE, Quantity INT CHECK(Quantity >= 0), Status TEXT CHECK(Status IN ('UNLOCKED', 'LOCKED', 'SPECIAL')));"
                "CREATE VIEW BarnCompacity AS SELECT SUM(Quantity) FROM Barn WHERE Status != 'SPECIAL';"
                "CREATE VIEW SiloCompacity AS SELECT SUM(Quantity) FROM Silo WHERE Status != 'SPECIAL';"
                "CREATE TABLE BarnMeta (Property TEXT UNIQUE, Value INT);"
                "CREATE TABLE SiloMeta (Property TEXT UNIQUE, Value INT);"
                "CREATE TABLE SkillTree (Skill TEXT UNIQUE, Status INT);"
                "CREATE TABLE EconContracts (Buyer TEXT, PRICE INT);"
                "CREATE TABLE Meta (Property TEXT UNIQUE, Value INT);";

    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

    sqlite3_close(db);

    return 0;
}

int open_save(const char* filename, sqlite3** db) {
    if (access(filename, F_OK)) {
        //return 1 if not exsits
        return 1;
    }

    int rc = sqlite3_open(filename, db);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(*db));
        return 1;
    }
    else {
        return 0;
    }
}

void close_save(sqlite3* db) {
    sqlite3_close(db);
}

int barn_query(sqlite3* db, const char* item) {
    sqlite3_stmt* stmt;

    int quanity;

    char* sql = "SELECT Quantity FROM Barn WHERE Item == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, item, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    quanity = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return quanity;
}

int silo_query(sqlite3* db, const char* item) {
    sqlite3_stmt* stmt;

    int quanity;

    char* sql = "SELECT Quantity FROM Silo WHERE Item == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, item, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    quanity = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return quanity;
}

int get_barn_allocation(sqlite3* db) {
    sqlite3_stmt* stmt;

    int allocation;

    char* sql = "SELECT * FROM BarnCompacity;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    allocation = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return allocation;
}

int get_silo_allocation(sqlite3* db) {
    sqlite3_stmt* stmt;

    int allocation;

    char* sql = "SELECT * FROM SiloCompacity;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    allocation = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return allocation;
}

int get_barn_max(sqlite3* db) {
    int max;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM BarnMeta WHERE Property == 'MaxCompacity';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    max = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return max;
}

int get_silo_max(sqlite3* db) {
    int max;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM SiloMeta WHERE Property == 'MaxCompacity';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    max = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return max;
}

enum item_status check_barn_item_status(sqlite3* db, const char* item) {
    sqlite3_stmt* stmt;

    const unsigned char* status_string;
    enum item_status status;

    char* sql = "SELECT Status FROM Barn WHERE Item == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, item, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    status_string = sqlite3_column_text(stmt, 0);

    sqlite3_finalize(stmt);

    if (sqlite3_stricmp( (char*) status_string, "UNLOCKED")) {
        status = UNLOCKED;
    }
    else if (sqlite3_stricmp( (char*) status_string, "LOCKED")) {
        status = LOCKED;
    }
    else if (sqlite3_stricmp( (char*) status_string, "SPECIAL")) {
        status = SPECIAL;
    }
    else {
        status = -1;
    }

    return status;
}

enum item_status check_silo_item_status(sqlite3* db, const char* item) {
    sqlite3_stmt* stmt;

    const unsigned char* status_string;
    enum item_status status;

    char* sql = "SELECT Status FROM Silo WHERE Item == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, item, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    status_string = sqlite3_column_text(stmt, 0);

    sqlite3_finalize(stmt);

    if (sqlite3_stricmp( (char*) status_string, "UNLOCKED")) {
        status = UNLOCKED;
    }
    else if (sqlite3_stricmp( (char*) status_string, "LOCKED")) {
        status = LOCKED;
    }
    else if (sqlite3_stricmp( (char*) status_string, "SPECIAL")) {
        status = SPECIAL;
    }
    else {
        status = -1;
    }

    return status;
}


int add_barn_meta_property(sqlite3* db, const char* property, const int key) {
    sqlite3_stmt* stmt;

    char* sql = "INSERT OR IGNORE INTO BarnMeta (Property, Value) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, property, -1, NULL);
        sqlite3_bind_int(stmt, 2, key);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int add_silo_meta_property(sqlite3* db, const char* property, const int key) {
    sqlite3_stmt* stmt;

    char* sql = "INSERT OR IGNORE INTO SiloMeta (Property, Value) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, property, -1, NULL);
        sqlite3_bind_int(stmt, 2, key);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int add_meta_property(sqlite3* db, const char* property, const int key) {
    sqlite3_stmt* stmt;

    char* sql = "INSERT OR IGNORE INTO Meta (Property, Value) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, property, -1, NULL);
        sqlite3_bind_int(stmt, 2, key);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int get_money(sqlite3* db) {
    int money;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM Meta WHERE Property == 'Money';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            return -1;
        }
    }

    money = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return money;
}

int get_level(sqlite3* db) {
    int level;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM Meta WHERE Property == 'Level';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    level = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return level;
}

int get_xp(sqlite3* db) {
    int xp;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM Meta WHERE Property == 'xp';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    xp = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return xp;
}

int get_number_of_fields(sqlite3* db) {
    int fields;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM Meta WHERE Property == 'Fields';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    fields = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return fields;
}

int get_number_of_tree_plots(sqlite3* db) {
    int plots;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM Meta WHERE Property == 'TreePlots';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    plots = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return plots;
}

int get_skill_points(sqlite3* db) {
    int points;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM Meta WHERE Property == 'SkillPoints';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    points = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return points;
}

int update_meta(sqlite3* db, const int added, const char* property) {
    sqlite3_stmt* stmt;

    char* sql = "UPDATE Meta SET Value = Value + ? WHERE Property == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, added);
        sqlite3_bind_text(stmt, 2, property, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int level_up(sqlite3* db, const int xp_needed) {

    if (update_meta(db, (-1 * xp_needed), "xp") != 0) {
        return -1;
    }

    if (update_meta(db, 1, "Level") != 0) {
        return -1;
    }

    return 0;
}

int add_to_skill_tree(sqlite3* db, const char* skill, const int status) {
    sqlite3_stmt* stmt;

    char* sql = "INSERT OR IGNORE INTO SkillTree (Skill, Status) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, skill, -1, NULL);
        sqlite3_bind_int(stmt, 2, status);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int update_skill_tree(sqlite3* db, const char* skill) {
    sqlite3_stmt* stmt;

    char* sql = "UPDATE SkillTree SET Status = Status + 1 WHERE Skill == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, skill, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int get_skill_status(sqlite3* db, const char* skill) {
    int status;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Status FROM SkillTree WHERE Skill == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, skill, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }

    status = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return status;
}

int update_barn(sqlite3* db, const char* item, const int changed) {
    sqlite3_stmt* stmt;

    char* sql = "UPDATE Barn SET Quantity = Quantity + ? WHERE Item == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, changed);
        sqlite3_bind_text(stmt, 2, item, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int update_silo(sqlite3* db, const char* item, const int changed) {
    sqlite3_stmt* stmt;

    char* sql = "UPDATE Silo SET Quantity = Quantity + ? WHERE Item == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, changed);
        sqlite3_bind_text(stmt, 2, item, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int update_barn_status(sqlite3* db, const char* item, const enum item_status status) {
    sqlite3_stmt* stmt;

    char* sql = "UPDATE Barn SET Status = ? WHERE Item == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {

        char* status_string = NULL;
        if (status == LOCKED) {
            status_string = "LOCKED";
        }
        else if (status == UNLOCKED) {
            status_string = "UNLOCKED";
        }
        else if (status == SPECIAL) {
            status_string = "SPECIAL";
        }
        sqlite3_bind_text(stmt, 1, status_string, -1, NULL);
        sqlite3_bind_text(stmt, 2, item, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int update_silo_status(sqlite3* db, const char* item, const enum item_status status) {
    sqlite3_stmt* stmt;

    char* sql = "UPDATE Silo SET Status = ? WHERE Item == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {

        char* status_string = NULL;
        if (status == LOCKED) {
            status_string = "LOCKED";
        }
        else if (status == UNLOCKED) {
            status_string = "UNLOCKED";
        }
        else if (status == SPECIAL) {
            status_string = "SPECIAL";
        }
        sqlite3_bind_text(stmt, 1, status_string, -1, NULL);
        sqlite3_bind_text(stmt, 2, item, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int add_item_to_barn(sqlite3* db, const char* item, const enum item_status status) {
    sqlite3_stmt* stmt;

    char* sql = "INSERT OR IGNORE INTO Barn (Item, Quantity, Status) VALUES (?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        char* status_string = NULL;
        if (status == LOCKED) {
            status_string = "LOCKED";
        }
        else if (status == UNLOCKED) {
            status_string = "UNLOCKED";
        }
        else if (status == SPECIAL) {
            status_string = "SPECIAL";
        }

        sqlite3_bind_text(stmt, 1, item, -1, NULL);
        sqlite3_bind_int(stmt, 2, 0);
        sqlite3_bind_text(stmt, 3, status_string, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int add_item_to_silo(sqlite3* db, const char* item, const enum item_status status) {
    sqlite3_stmt* stmt;

    char* sql = "INSERT OR IGNORE INTO Silo (Item, Quantity, Status) VALUES (?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        char* status_string = NULL;
        if (status == LOCKED) {
            status_string = "LOCKED";
        }
        else if (status == UNLOCKED) {
            status_string = "UNLOCKED";
        }
        else if (status == SPECIAL) {
            status_string = "SPECIAL";
        }
        
        sqlite3_bind_text(stmt, 1, item, -1, NULL);
        sqlite3_bind_int(stmt, 2, 0);
        sqlite3_bind_text(stmt, 3, status_string, -1, NULL);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}
