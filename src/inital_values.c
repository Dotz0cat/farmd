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

#include "inital_values.h"

int add_inital_save_values(sqlite3 *db) {
    int rc = 0;

    struct table {
        char *name;
        int inital_value;
    };

    struct table storage_meta[] = {
        {"Level", 1},
        {"MaxCapacity", 50},
    };

    for (int i = 0; i < (int) (sizeof(storage_meta) / sizeof(storage_meta[0])); i++) {
        rc = add_barn_meta_property(db, storage_meta[i].name, storage_meta[i].inital_value);
        if (rc != 0) {
            return -1;
        }
        rc = add_silo_meta_property(db, storage_meta[i].name, storage_meta[i].inital_value);
        if (rc != 0) {
            return -1;
        }
    }

    struct table processing_building_meta[] = {
        {"Level", 0},
        {"Queues", 0},
        {"SlotsInQueue0", 0},
    };

    for (int i = 0; i < (int) (sizeof(processing_building_meta) / sizeof(processing_building_meta[0])); i++) {
        rc = add_grain_mill_meta_property(db, processing_building_meta[i].name, processing_building_meta[i].inital_value);
        if (rc != 0) {
            return -1;
        }
    }

    struct table meta_values[] = {
        {"Money", 1000},
        {"Level", 1},
        {"xp", 0},
        {"SkillPoints", 10},
        {"Fields", 0},
        {"TreePlots", 0},
        {"GrainMill", 0},
    };

    for (int i = 0; i < (int) (sizeof(meta_values) / sizeof(meta_values[0])); i++) {
        rc = add_meta_property(db, meta_values[i].name, meta_values[i].inital_value);
        if (rc != 0) {
            return -1;
        }
    }

    struct table skill_tree[] = {
        {"Farming", 0},
        {"Fields", 0},
        {"wheat", 0},
        {"corn", 0},
        {"potatos", 0},
        {"beets", 0},
        {"sugarcane", 0},
        {"turnips", 0},
        {"tomatos", 0},
        {"cucumbers", 0},
        {"okra", 0},
        {"TreePlots", 0},
        {"pears", 0},
        {"apples", 0},
        {"oranges", 0},
        {"peaches", 0},
        {"LiveStock", 0},
        {"Processing", 0},
        {"Dairy", 0},
        {"pasteurized", 0},
        {"butter", 0},
        {"cheese", 0},
        {"FeedMill", 0},
        {"cowFeed", 0},
        {"chickenFeed", 0},
        {"Milling", 0},
        {"GrainMill", 0},
        {"wheatFlour", 0},
        {"cornMeal", 0},
        {"SugarMill", 0},
        {"beetSugar", 0},
        {"caneSugar", 0},
    };

    for (int i = 0; i < (int) (sizeof(skill_tree) / sizeof(skill_tree[0])); i++) {
        rc = add_to_skill_tree(db, skill_tree[i].name, skill_tree[i].inital_value);
        if (rc != 0) {
            return -1;
        }
    }

    return 0;
}
