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

#ifndef SAVE_H
#define SAVE_H

#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>

#include <sqlite3.h>

enum item_status {
    UNLOCKED,
    LOCKED,
    SPECIAL
};

int create_save_db(const char* filename);
int open_save_db(const char* filename, sqlite3** db);
void close_save_db(sqlite3* db);

int barn_query_db(sqlite3* db, const char* item);
int silo_query_db(sqlite3* db, const char* item);

int get_barn_allocation(sqlite3* db);
int get_silo_allocation(sqlite3* db);

int get_barn_max(sqlite3* db);
int get_silo_max(sqlite3* db);

enum item_status check_barn_item_status(sqlite3* db, const char* item);
enum item_status check_silo_item_status(sqlite3* db, const char* item);

int add_barn_meta_property(sqlite3* db, const char* property, const int key);
int add_silo_meta_property(sqlite3* db, const char* property, const int key);

int get_barn_meta_property(sqlite3* db, const char* property);
int get_silo_meta_property(sqlite3* db, const char* property);

int update_barn_meta_property(sqlite3* db, const char* property, const int value);
int update_silo_meta_property(sqlite3* db, const char* property, const int value);

int add_meta_property(sqlite3* db, const char* property, const int key);

int get_money(sqlite3* db);
int get_level(sqlite3* db);
int get_xp(sqlite3* db);
int get_number_of_fields(sqlite3* db);
int get_number_of_tree_plots(sqlite3* db);
int get_skill_points(sqlite3* db);

int update_meta(sqlite3* db, const int added, const char* property);

int level_up(sqlite3* db, const int xp_needed);

int add_to_skill_tree(sqlite3* db, const char* skill, const int status);
int update_skill_tree(sqlite3* db, const char* skill);
int get_skill_status(sqlite3* db, const char* skill);

int update_barn(sqlite3* db, const char* item, const int changed);
int update_silo(sqlite3* db, const char* item, const int changed);
int update_barn_status(sqlite3* db, const char* item, const enum item_status status);
int update_silo_status(sqlite3* db, const char* item, const enum item_status status);
int add_item_to_barn(sqlite3* db, const char* item, const enum item_status status);
int add_item_to_silo(sqlite3* db, const char* item, const enum item_status status);

int add_tree(sqlite3* db, const int index);
int remove_tree(sqlite3* db, const int index);
int set_tree_type(sqlite3* db, const int index, const char* type);
const char* get_tree_type(sqlite3* db, const int tree_number);

int set_tree_time(sqlite3* db, const int index, const time_t time);
time_t get_tree_time(sqlite3* db, const int index);
int clear_tree_time(sqlite3* db, const int index);
int set_tree_completion(sqlite3* db, const int index, const int completion);
int get_tree_completion(sqlite3* db, const int index);

int set_tree_maturity(sqlite3* db, const int index, const int maturity);
int get_tree_maturity(sqlite3* db, const int index);

int add_field(sqlite3* db, const int index);
int remove_field(sqlite3* db, const int index);
int set_field_type(sqlite3* db, const int index, const char* type);
const char* get_field_type(sqlite3* db, const int field_number);

int set_field_time(sqlite3* db, const int index, const time_t time);
time_t get_field_time(sqlite3* db, const int index);
int clear_field_time(sqlite3* db, const int index);
int set_field_completion(sqlite3* db, const int index, const int completion);
int get_field_completion(sqlite3* db, const int index);

#endif /* SAVE_H */

/*
CREATE TABLE Barn (Item TEXT UNIQUE, Quantity INT CHECK(Quantity >= 0), Status TEXT CHECK(Status IN ('UNLOCKED', 'LOCKED', 'SPECIAL')));
CREATE TABLE Silo (Item TEXT UNIQUE, Quantity INT CHECK(Quantity >= 0), Status TEXT CHECK(Status IN ('UNLOCKED', 'LOCKED', 'SPECIAL')));
                    
    CREATE VIEW BarnCapacity AS
    SELECT SUM(Quanity) FROM Barn
    WHERE Status != 'SPECIAL';
                    
    CREATE VIEW SiloCapacity AS
    SELECT SUM(Quanity) FROM Silo
    WHERE Status != 'SPECIAL';
    
CREATE TABLE BarnMeta (Property TEXT UNIQUE, Value INT CHECK(Value >= 0));
CREATE TABLE SiloMeta (Property TEXT UNIQUE, Value INT CHECK(Value >= 0));

CREATE TABLE SkillTree (Skill TEXT UNIQUE, Status INT CHECK(Status >= 0));

CREATE TABLE EconContracts (Buyer TEXT, Item TEXT, ItemReward TEXT, Reward INT CHECK(Reward > 0));

CREATE TABLE Meta (Property TEXT UNIQUE, Value INT CHECK(Value >= 0));

CREATE TABLE Trees (TreeIndex INT UNIQUE, Type TEXT, Mature INT CHECK(Mature = 0 OR Mature = 1), Completion INT CHECK(Completion = 0 OR Completion = 1), Time INT);

CREATE TABLE Fields (FieldIndex INT UNIQUE, Type TEXT, Completion INT CHECK(Completion = 0 OR Completion = 1), Time INT);
*/
