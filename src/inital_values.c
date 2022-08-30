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

#include "inital_values.h"

int add_inital_save_values(sqlite3* db) {

    int rc;
    rc = add_barn_meta_property(db, "Level", 1);
    if (rc != 0) {
        return -1;
    }

    rc = add_silo_meta_property(db, "Level", 1);
    if (rc != 0) {
        return -1;
    }

    rc = add_barn_meta_property(db, "MaxCompacity", 50);
    if (rc != 0) {
        return -1;
    }

    rc = add_silo_meta_property(db, "MaxCompacity", 50);
    if (rc != 0) {
        return -1;
    }

    rc = add_meta_property(db, "Money", 1000);
    if (rc != 0) {
        return -1;
    }

    rc = add_meta_property(db, "Level", 1);
    if (rc != 0) {
        return -1;
    }

    rc = add_meta_property(db, "xp", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_meta_property(db, "SkillPoints", 10);
    if (rc != 0) {
        return -1;
    }

    rc = add_meta_property(db, "Fields", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_meta_property(db, "TreePlots", 0);
    if (rc != 0) {
        return -1;
    }

    //farming branch
    rc = add_to_skill_tree(db, "Farming", 0);
    if (rc != 0) {
        return -1;
    }

    //number of fields that can be bought 3 for each level
    rc = add_to_skill_tree(db, "Fields", 0);
    if (rc != 0) {
        return -1;
    }

    //crop types unlock once
    rc = add_to_skill_tree(db, "wheat", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "corn", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "potatos", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "beets", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "sugarcane", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "turnips", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "tomatos", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "cucumbers", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "okra", 0);
    if (rc != 0) {
        return -1;
    }

    //plots for trees that can be bought 1x
    rc = add_to_skill_tree(db, "TreePlots", 0);
    if (rc != 0) {
        return -1;
    }

    //tree types bought once
    rc = add_to_skill_tree(db, "pears", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "apples", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "oranges", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "peaches", 0);
    if (rc != 0) {
        return -1;
    }

    //livestock branch
    rc = add_to_skill_tree(db, "LiveStock", 0);
    if (rc != 0) {
        return -1;
    }

    //processing branch
    rc = add_to_skill_tree(db, "Processing", 0);
    if (rc != 0) {
        return -1;
    }

    //only one for buildings
    rc = add_to_skill_tree(db, "Dairy", 0);
    if (rc != 0) {
        return -1;
    }

    //dairy product types
    rc = add_to_skill_tree(db, "pasterized", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "butter", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "cheese", 0);
    if (rc != 0) {
        return -1;
    }

    //building
    rc = add_to_skill_tree(db, "FeedMill", 0);
    if (rc != 0) {
        return -1;
    }

    //feed types
    rc = add_to_skill_tree(db, "cowFeed", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "chickenFeed", 0);
    if (rc != 0) {
        return -1;
    }

    //building type (milling sub branch)
    rc = add_to_skill_tree(db, "Milling", 0);
    if (rc != 0) {
        return -1;
    }

    //building
    rc = add_to_skill_tree(db, "GrainMill", 0);
    if (rc != 0) {
        return -1;
    }

    //types
    rc = add_to_skill_tree(db, "wheatFlour", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "cornMeal", 0);
    if (rc != 0) {
        return -1;
    }

    //building
    rc = add_to_skill_tree(db, "SugarMill", 0);
    if (rc != 0) {
        return -1;
    }

    //types
    rc = add_to_skill_tree(db, "beetSugar", 0);
    if (rc != 0) {
        return -1;
    }

    rc = add_to_skill_tree(db, "caneSugar", 0);
    if (rc != 0) {
        return -1;
    }

    return 0;
}
