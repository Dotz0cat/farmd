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
        //will likely put in syslog
        //fprintf(stderr, "SQLite3 error: %s\r\n", sqlite3_errmsg(db));
        //fprintf(stderr, "SQLITE3 error info: %i\r\n", sqlite3_system_errno(db));

        sqlite3_close(db);

        return 1;
    }

    char* sql = "CREATE TABLE Barn (Item TEXT, Quantity INT, Status TEXT CHECK(Status IN ('UNLOCKED', 'LOCKED', 'SPECIAL')));"
                "CREATE TABLE Silo (Item TEXT, Quantity INT, Status TEXT CHECK(Status IN ('UNLOCKED', 'LOCKED', 'SPECIAL')));"
                "CREATE VIEW BarnCompacity AS SELECT SUM(Quantity) FROM Barn WHERE Status != 'SPECIAL';"
                "CREATE VIEW SiloCompacity AS SELECT SUM(Quantity) FROM Silo WHERE Status != 'SPECIAL';"
                "CREATE TABLE BarnMeta (Property TEXT, Value INT);"
                "CREATE TABLE SiloMeta (Property TEXT, Value INT);"
                "CREATE TABLE SkillTree (Skill TEXT, Status INT);"
                "CREATE TABLE EconContracts (Buyer TEXT, PRICE INT);"
                "CREATE TABLE Meta (Property TEXT, Value INT);";

    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);

    if (rc != SQLITE_OK) {
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
        return 1;
    }
    else {
        return 0;
    }
}

void close_save(sqlite3* db) {
    sqlite3_close(db);
    db = NULL;
}

int barn_query(sqlite3* db, const char* item) {
    sqlite3_stmt* stmt;

    int quanity;

    char* sql = "SELECT Quantity FROM Barn WHERE Item == ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, item, -1, NULL);
    }
    else {

        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
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

    char* sql = "SELECT Quantity FROM Silo WHERE Item == ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, item, -1, NULL);
    }
    else {

        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
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
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
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
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
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
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
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
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
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

    char* sql = "SELECT Status FROM Barn WHERE Item == ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, item, -1, NULL);
    }
    else {

        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
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

    char* sql = "SELECT Status FROM Silo WHERE Item == ?";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, item, -1, NULL);
    }
    else {

        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        if (rc != SQLITE_ROW) {
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

    char* sql = "INSERT INTO BarnMeta (Property, Value) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, property, -1, NULL);
        sqlite3_bind_int(stmt, 2, key);
    }
    else {
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);

        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int add_silo_meta_property(sqlite3* db, const char* property, const int key) {
    sqlite3_stmt* stmt;

    char* sql = "INSERT INTO SiloMeta (Property, Value) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, property, -1, NULL);
        sqlite3_bind_int(stmt, 2, key);
    }
    else {
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int add_meta_property(sqlite3* db, const char* property, const int key) {
    sqlite3_stmt* stmt;

    char* sql = "INSERT INTO Meta (Property, Value) VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, property, -1, NULL);
        sqlite3_bind_int(stmt, 2, key);
    }
    else {
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int get_money(sqlite3* db) {
    int money;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM Meta WHERE Property == Money";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return -1;
    }

    money = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return money;
}

int get_level(sqlite3* db) {
    int level;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM Meta WHERE Property == Level";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return -1;
    }

    level = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return level;
}

int get_xp(sqlite3* db) {
    int xp;

    sqlite3_stmt* stmt;

    char* sql = "SELECT Value FROM Meta WHERE Property == xp";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return -1;
    }

    xp = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return xp;
}

int update_xp(sqlite3* db, int added) {
    sqlite3_stmt* stmt;

    char* sql = "UPDATE Meta SET Value = (SELECT Value FROM Meta WHERE Property == 'xp') + ? WHERE Property == 'xp';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, added);
    }
    else {
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK && rc != SQLITE_DONE) {
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int update_money(sqlite3* db, int added) {
    sqlite3_stmt* stmt;

    char* sql = "UPDATE Meta SET Value = (SELECT Value FROM Meta WHERE Property == 'Money') + ? WHERE Property == 'Money';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, added);
    }
    else {
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_OK) {
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int level_up(sqlite3* db) {
     sqlite3_stmt* stmt;

    char* sql = "UPDATE Meta SET Value = 0 WHERE Property == xp;"
                "UPDATE Meta SET Value = (SELECT Value FROM Meta WHERE Property == 'Level') + 1 WHERE Property == 'level';";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);

        return -1;
    }

    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}
