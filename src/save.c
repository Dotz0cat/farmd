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

#include "save.h"

int create_save_db(const char *filename) {
    sqlite3 *db;

    int rc = sqlite3_open(filename, &db);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));

        sqlite3_close(db);

        return 1;
    }

    char *sql = "CREATE TABLE Barn (Item TEXT UNIQUE, Quantity INT CHECK(Quantity >= 0), Status TEXT CHECK(Status IN ('UNLOCKED', 'LOCKED', 'SPECIAL')));"
                "CREATE TABLE Silo (Item TEXT UNIQUE, Quantity INT CHECK(Quantity >= 0), Status TEXT CHECK(Status IN ('UNLOCKED', 'LOCKED', 'SPECIAL')));"
                "CREATE VIEW BarnCapacity AS SELECT SUM(Quantity) FROM Barn WHERE Status != 'SPECIAL';"
                "CREATE VIEW SiloCapacity AS SELECT SUM(Quantity) FROM Silo WHERE Status != 'SPECIAL';"
                "CREATE TABLE BarnMeta (Property TEXT UNIQUE, Value INT CHECK(Value >= 0));"
                "CREATE TABLE SiloMeta (Property TEXT UNIQUE, Value INT CHECK(Value >= 0));"
                "CREATE TABLE SkillTree (Skill TEXT UNIQUE, Status INT CHECK(Status >= 0));"
                "CREATE TABLE EconContracts (Buyer TEXT, Price INT CHECK(Price > 0));"
                "CREATE TABLE Meta (Property TEXT UNIQUE, Value INT CHECK(Value >= 0));"
                "CREATE TABLE Trees (TreeIndex INT UNIQUE, Type TEXT, Mature INT CHECK(Mature = 0 OR Mature = 1), Completion INT CHECK(Completion = 0 OR Completion = 1), Time INT);"
                "CREATE TABLE Fields (FieldIndex INT UNIQUE, Type TEXT, Completion INT CHECK(Completion = 0 OR Completion = 1), Time INT);"
                "CREATE TABLE GrainMill (QueueIndex INT, SlotIndex INT, Type TEXT, Completion INT CHECK(Completion = 0 OR Completion = 1), StartTime INT, EndTime INT CHECK(EndTime > StartTime));"
                "CREATE UNIQUE INDEX GrainMill_QS ON GrainMill(QueueIndex, SlotIndex);"
                "CREATE TABLE GrainMillMeta (Property TEXT UNIQUE, Value INT CHECK(Value >= 0));";

    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);

    if (rc != SQLITE_OK) {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

    sqlite3_close(db);

    return 0;
}

int open_save_db(const char *filename, sqlite3 **db) {
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

void close_save_db(sqlite3 *db) {
    sqlite3_close(db);
}

int barn_query_db(sqlite3 *db, const char *item) {
    sqlite3_stmt *stmt;

    int quanity;

    char *sql = "SELECT Quantity FROM Barn WHERE Item == ?;";

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

int silo_query_db(sqlite3 *db, const char *item) {
    sqlite3_stmt *stmt;

    int quanity;

    char *sql = "SELECT Quantity FROM Silo WHERE Item == ?;";

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

int get_barn_allocation(sqlite3 *db) {
    sqlite3_stmt *stmt;

    int allocation;

    char *sql = "SELECT * FROM BarnCapacity;";

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

int get_silo_allocation(sqlite3 *db) {
    sqlite3_stmt *stmt;

    int allocation;

    char *sql = "SELECT * FROM SiloCapacity;";

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

int get_barn_max(sqlite3 *db) {
    int max;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM BarnMeta WHERE Property == 'MaxCapacity';";

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

int get_silo_max(sqlite3 *db) {
    int max;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM SiloMeta WHERE Property == 'MaxCapacity';";

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

enum item_status check_barn_item_status(sqlite3 *db, const char *item) {
    sqlite3_stmt *stmt;

    const unsigned char *status_string;
    enum item_status status;

    char *sql = "SELECT Status FROM Barn WHERE Item == ?;";

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

    if (sqlite3_stricmp( (char*) status_string, "UNLOCKED") == 0) {
        status = UNLOCKED;
    }
    else if (sqlite3_stricmp( (char*) status_string, "LOCKED") == 0) {
        status = LOCKED;
    }
    else if (sqlite3_stricmp( (char*) status_string, "SPECIAL") == 0) {
        status = SPECIAL;
    }
    else {
        status = -1;
    }

    sqlite3_finalize(stmt);

    return status;
}

enum item_status check_silo_item_status(sqlite3 *db, const char *item) {
    sqlite3_stmt *stmt;

    const unsigned char *status_string;
    enum item_status status;

    char *sql = "SELECT Status FROM Silo WHERE Item == ?;";

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

    if (sqlite3_stricmp( (char*) status_string, "UNLOCKED") == 0) {
        status = UNLOCKED;
    }
    else if (sqlite3_stricmp( (char*) status_string, "LOCKED") == 0) {
        status = LOCKED;
    }
    else if (sqlite3_stricmp( (char*) status_string, "SPECIAL") == 0) {
        status = SPECIAL;
    }
    else {
        status = -1;
    }

    sqlite3_finalize(stmt);

    return status;
}


int add_barn_meta_property(sqlite3 *db, const char *property, const int key) {
    sqlite3_stmt *stmt;

    char *sql = "INSERT OR IGNORE INTO BarnMeta (Property, Value) VALUES (?, ?);";

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

int add_silo_meta_property(sqlite3 *db, const char *property, const int key) {
    sqlite3_stmt *stmt;

    char *sql = "INSERT OR IGNORE INTO SiloMeta (Property, Value) VALUES (?, ?);";

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

int get_barn_meta_property(sqlite3 *db, const char *property) {
    int value;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM BarnMeta WHERE Property == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, property, -1, NULL);
       
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

    value = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return value;
}

int get_silo_meta_property(sqlite3 *db, const char *property) {
    int value;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM SiloMeta WHERE Property == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, property, -1, NULL);
       
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

    value = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return value;
}

int update_barn_meta_property(sqlite3 *db, const char *property, const int value) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE BarnMeta SET Value = Value + ? WHERE Property == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, value);
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

int update_silo_meta_property(sqlite3 *db, const char *property, const int value) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE SiloMeta SET Value = Value + ? WHERE Property == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, value);
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

int add_meta_property(sqlite3 *db, const char *property, const int key) {
    sqlite3_stmt *stmt;

    char *sql = "INSERT OR IGNORE INTO Meta (Property, Value) VALUES (?, ?);";

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

int get_money(sqlite3 *db) {
    int money;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM Meta WHERE Property == 'Money';";

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

int get_level(sqlite3 *db) {
    int level;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM Meta WHERE Property == 'Level';";

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

int get_xp(sqlite3 *db) {
    int xp;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM Meta WHERE Property == 'xp';";

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

int get_number_of_fields(sqlite3 *db) {
    int fields;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM Meta WHERE Property == 'Fields';";

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

int get_number_of_tree_plots(sqlite3 *db) {
    int plots;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM Meta WHERE Property == 'TreePlots';";

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

int get_skill_points(sqlite3 *db) {
    int points;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM Meta WHERE Property == 'SkillPoints';";

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

int get_grain_mill_bought_status(sqlite3 *db) {
    int points;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM Meta WHERE Property == 'GrainMill';";

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

int update_meta(sqlite3 *db, const int added, const char *property) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Meta SET Value = Value + ? WHERE Property == ?;";

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

int level_up(sqlite3 *db, const int xp_needed) {

    if (update_meta(db, (-1 * xp_needed), "xp") != 0) {
        return -1;
    }

    if (update_meta(db, 1, "Level") != 0) {
        return -1;
    }

    return 0;
}

int add_to_skill_tree(sqlite3 *db, const char *skill, const int status) {
    sqlite3_stmt *stmt;

    char *sql = "INSERT OR IGNORE INTO SkillTree (Skill, Status) VALUES (?, ?);";

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

int update_skill_tree(sqlite3 *db, const char *skill) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE SkillTree SET Status = Status + 1 WHERE Skill == ?;";

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

int get_skill_status(sqlite3 *db, const char *skill) {
    int status;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Status FROM SkillTree WHERE Skill == ?;";

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

int update_barn(sqlite3 *db, const char *item, const int changed) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Barn SET Quantity = Quantity + ? WHERE Item == ?;";

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

int update_silo(sqlite3 *db, const char *item, const int changed) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Silo SET Quantity = Quantity + ? WHERE Item == ?;";

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

int update_barn_status(sqlite3 *db, const char *item, const enum item_status status) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Barn SET Status = ? WHERE Item == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {

        char *status_string = NULL;
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

int update_silo_status(sqlite3 *db, const char *item, const enum item_status status) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Silo SET Status = ? WHERE Item == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {

        char *status_string = NULL;
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

int add_item_to_barn(sqlite3 *db, const char *item, const enum item_status status) {
    sqlite3_stmt *stmt;

    char *sql = "INSERT OR IGNORE INTO Barn (Item, Quantity, Status) VALUES (?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        char *status_string = NULL;
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

int add_item_to_silo(sqlite3 *db, const char *item, const enum item_status status) {
    sqlite3_stmt *stmt;

    char *sql = "INSERT OR IGNORE INTO Silo (Item, Quantity, Status) VALUES (?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        char *status_string = NULL;
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

int add_tree(sqlite3 *db, const int index) {
    sqlite3_stmt *stmt;

    char *sql = "INSERT OR IGNORE INTO Trees (TreeIndex, Type, Completion) VALUES (?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
        sqlite3_bind_text(stmt, 2, "none", -1, NULL);
        sqlite3_bind_int(stmt, 3, 0);
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

int remove_tree(sqlite3 *db, const int index) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Trees SET Type = 'none' WHERE TreeIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
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

int set_tree_type(sqlite3 *db, const int index, const char *type) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Trees SET Type = ? WHERE TreeIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, type, -1, NULL);
        sqlite3_bind_int(stmt, 2, index);
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

char *get_tree_type(sqlite3 *db, const int tree_number) {
    sqlite3_stmt *stmt;

    const unsigned char *tree_type = NULL;
    char *type = NULL;

    char *sql = "SELECT Type FROM Trees WHERE TreeIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, tree_number);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return NULL;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return NULL;
        }
    }

    tree_type = sqlite3_column_text(stmt, 0);

    type = strdup( (const char*) tree_type);

    sqlite3_finalize(stmt);

    return type;
}

int set_tree_time(sqlite3 *db, const int index, const time_t time) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Trees SET Time = UNIXEPOCH() + ? WHERE TreeIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, time);
        sqlite3_bind_int(stmt, 2, index);
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

time_t get_tree_time(sqlite3 *db, const int index) {
    time_t time;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Time FROM Trees WHERE TreeIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
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

    time = sqlite3_column_int64(stmt, 0);

    sqlite3_finalize(stmt);

    return time;
}

int clear_tree_time(sqlite3 *db, const int index) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Trees SET Time = NULL WHERE TreeIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
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

int set_tree_completion(sqlite3 *db, const int index, const int completion) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Trees SET Completion = ? WHERE TreeIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, completion);
        sqlite3_bind_int(stmt, 2, index);
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

int get_tree_completion(sqlite3 *db, const int index) {
    int complete;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Completion FROM Trees WHERE TreeIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
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

    complete = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return complete;
}

int set_tree_maturity(sqlite3 *db, const int index, const int maturity) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Trees SET Mature = ? WHERE TreeIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, maturity);
        sqlite3_bind_int(stmt, 2, index);
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

int get_tree_maturity(sqlite3 *db, const int index) {
    int mature;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Mature FROM Trees WHERE TreeIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
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

    mature = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return mature;
}

int add_field(sqlite3 *db, const int index) {
    sqlite3_stmt *stmt;

    char *sql = "INSERT OR IGNORE INTO Fields (FieldIndex, Type, Completion) VALUES (?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
        sqlite3_bind_text(stmt, 2, "none", -1, NULL);
        sqlite3_bind_int(stmt, 3, 0);
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

int remove_field(sqlite3 *db, const int index) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Fields SET Type = 'none' WHERE FieldIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
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

int set_field_type(sqlite3 *db, const int index, const char *type) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Fields SET Type = ? WHERE FieldIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, type, -1, NULL);
        sqlite3_bind_int(stmt, 2, index);
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

char *get_field_type(sqlite3 *db, const int field_number) {
    sqlite3_stmt *stmt;

    const unsigned char *field_type = NULL;
    char *type = NULL;

    char *sql = "SELECT Type FROM Fields WHERE FieldIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, field_number);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return NULL;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return NULL;
        }
    }

    field_type = sqlite3_column_text(stmt, 0);

    type = strdup( (const char*) field_type);

    sqlite3_finalize(stmt);

    return type;
}

int set_field_time(sqlite3 *db, const int index, const time_t time) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Fields SET Time = UNIXEPOCH() + ? WHERE FieldIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, time);
        sqlite3_bind_int(stmt, 2, index);
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

time_t get_field_time(sqlite3 *db, const int index) {
    time_t time;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Time FROM Fields WHERE FieldIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
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

    time = sqlite3_column_int64(stmt, 0);

    sqlite3_finalize(stmt);

    return time;
}

int clear_field_time(sqlite3 *db, const int index) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Fields SET Time = NULL WHERE FieldIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
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

int set_field_completion(sqlite3 *db, const int index, const int completion) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE Fields SET Completion = ? WHERE FieldIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, completion);
        sqlite3_bind_int(stmt, 2, index);
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

int get_field_completion(sqlite3 *db, const int index) {
    int complete;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Completion FROM Fields WHERE FieldIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, index);
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

    complete = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return complete;
}

int add_slot_to_grain_mill(sqlite3 *db, const int queue_index, const int slot_index) {
    sqlite3_stmt *stmt;

    char *sql = "INSERT OR IGNORE INTO GrainMill (QueueIndex, SlotIndex, Type, Completion) VALUES (?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, queue_index);
        sqlite3_bind_int(stmt, 2, slot_index);
        sqlite3_bind_text(stmt, 3, "none", -1, NULL);
        sqlite3_bind_int(stmt, 4, 0);
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

int set_type_grain_mill(sqlite3 *db, const int queue_index, const int slot_index, const char *type) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE GrainMill SET Type = ? WHERE QueueIndex == ? AND SlotIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, type, -1, NULL);
        sqlite3_bind_int(stmt, 2, slot_index);
        sqlite3_bind_int(stmt, 3, queue_index);
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

char *get_grain_mill_type(sqlite3 *db, const int queue_index, const int slot_index) {
    sqlite3_stmt *stmt;

    const unsigned char *slot_type = NULL;
    char *type = NULL;

    char *sql = "SELECT Type FROM GrainMill WHERE QueueIndex == ? AND SlotIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, queue_index);
        sqlite3_bind_int(stmt, 2, slot_index);
    }
    else {
        syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);

        return NULL;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
            syslog(LOG_WARNING, "sqlite3 error: %s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return NULL;
        }
    }

    slot_type = sqlite3_column_text(stmt, 0);

    type = strdup( (const char*) slot_type);

    sqlite3_finalize(stmt);

    return type;
}

int set_grain_mill_end_time(sqlite3 *db, const int queue_index, const int slot_index, const time_t time) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE GrainMill SET EndTime = ? WHERE QueueIndex == ? AND SlotIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, time);
        sqlite3_bind_int(stmt, 2, queue_index);
        sqlite3_bind_int(stmt, 3, slot_index);
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

time_t get_grain_mill_end_time(sqlite3 *db, const int queue_index, const int slot_index) {
    time_t time;

    sqlite3_stmt *stmt;

    char *sql = "SELECT EndTime FROM GrainMill WHERE QueueIndex == ? AND SlotIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, queue_index);
        sqlite3_bind_int(stmt, 2, slot_index);
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

    time = sqlite3_column_int64(stmt, 0);

    sqlite3_finalize(stmt);

    return time;
}

int set_grain_mill_start_time(sqlite3 *db, const int queue_index, const int slot_index, const time_t time) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE GrainMill SET StartTime = ? WHERE QueueIndex == ? AND SlotIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, time);
        sqlite3_bind_int(stmt, 2, queue_index);
        sqlite3_bind_int(stmt, 3, slot_index);
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

time_t get_grain_mill_start_time(sqlite3 *db, const int queue_index, const int slot_index) {
    time_t time;

    sqlite3_stmt *stmt;

    char *sql = "SELECT StartTime FROM GrainMill WHERE QueueIndex == ? AND SlotIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, queue_index);
        sqlite3_bind_int(stmt, 2, slot_index);
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

    time = sqlite3_column_int64(stmt, 0);

    sqlite3_finalize(stmt);

    return time;
}

int clear_grain_mill_time(sqlite3 *db, const int queue_index, const int slot_index) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE GrainMill SET StartTime = NULL, EndTime = NULL WHERE QueueIndex == 0 AND SlotIndex == 1;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, queue_index);
        sqlite3_bind_int(stmt, 2, slot_index);
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

int set_grain_mill_completion(sqlite3 *db, const int queue_index, const int slot_index, const int completion) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE GrainMill SET Completion = ? WHERE QueueIndex == ? AND SlotIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, completion);
        sqlite3_bind_int(stmt, 2, queue_index);
        sqlite3_bind_int(stmt, 3, slot_index);
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

int get_grain_mill_completion(sqlite3 *db, const int queue_index, const int slot_index) {
    int complete;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Completion FROM GrainMill WHERE QueueIndex == ? AND SlotIndex == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, queue_index);
        sqlite3_bind_int(stmt, 2, slot_index);
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

    complete = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return complete;
}

int add_grain_mill_meta_property(sqlite3 *db, const char *property, const int value) {
    sqlite3_stmt *stmt;

    char *sql = "INSERT OR IGNORE INTO GrainMillMeta (Property, Value) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, property, -1, NULL);
        sqlite3_bind_int(stmt, 2, value);
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

int get_grain_mill_meta_property(sqlite3 *db, const char *property) {
    int value;

    sqlite3_stmt *stmt;

    char *sql = "SELECT Value FROM GrainMillMeta WHERE Property == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, property, -1, NULL);
       
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

    value = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return value;
}

int update_grain_mill_meta_property(sqlite3 *db, const char *property, const int value) {
    sqlite3_stmt *stmt;

    char *sql = "UPDATE GrainMillMeta SET Value = Value + ? WHERE Property == ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, value);
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
