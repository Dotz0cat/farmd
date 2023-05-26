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
    rc = add_to_skill_tree(db, "pasteurized", 0);
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

const char* skill_dep_check(sqlite3* db, const char* skill) {
    switch (hash(skill)) {
        //Fields
        case (753281362039030180ul):
        //wheat
        case (7449900282074400153ul):
        //corn
        case (9054106518560669730ul):
        //potatos
        case (7877483822930927482ul):
        //beets
        case (6159784693414617594ul):
        //sugarcane
        case (9644926710613211612ul):
        //turnips
        case (7259940842335963470ul):
        //tomatos
        case (11580180853258389223ul):
        //cucumbers
        case (1675513966417282617ul):
        //okra
        case (6301598470836844819ul):
        //TreePlots
        case (8178761559116963079ul):
        //pears
        case (5396648308222667934ul):
        //apples
        case (18333818697599901006ul):
        //oranges
        case (10410096555382471531ul):
        //peaches
        case (6920670729481494599ul):
            //Farming branch
            if (get_skill_status(db, "Farming") != 0) {
                return NULL;
            }
            else {
                return "Dependancy needed: Farming";
            }
        break;
        //nop till populated
        case (1185095156092737501ul):
            //lLivestock branch
            if (get_skill_status(db, "LiveStock") != 0) {
                return NULL;
            }
            else {
                return "Dependancy needed: LiveStock";
            }
        break;
        //pasteurized
        case (8415674580895657090ul):
        //butter
        case (17300743435645978621ul):
        //cheese
        case (7423936227120320490ul):
            //dairy processing branch
            if (get_skill_status(db, "Dairy") != 0) {
                return NULL;
            }
            else {
                return "Dependancy needed: Dairy";
            }
        break;
        //cowFeed
        case (647418952940708253ul):
        //chickenFeed
        case (3268618862269003574ul):
            //feed mill processing branch
            if (get_skill_status(db, "FeedMill") != 0) {
                return NULL;
            }
            else {
                return "Dependancy needed: FeedMill";
            }
        break;
        //GrainMill
        case (9636537473772185022ul):
        //SugarMill
        case (15740060735858632559ul):
            //milling processing branch
            if (get_skill_status(db, "Milling") != 0) {
                return NULL;
            }
            else {
                return "Dependancy needed: Milling";
            }
        break;
        //wheatFlour
        case (9929805013120973397ul):
        //cornMeal
        case (17229215621694050128ul):
            //grain mill processing branch
            if (get_skill_status(db, "GrainMill") != 0) {
                return NULL;
            }
            else {
                return "Dependancy needed: GrainMill";
            }
        break;
        //caneSugar
        case (5979901262968083966ul):
        //beetSugar
        case (18148427013753227637ul):
            //sugar mill processing branch
            if (get_skill_status(db, "SugarMill") != 0) {
                return NULL;
            }
            else {
                return "Dependancy needed: SugarMill";
            }
        break;
        //Dairy
        case (504030753684137974ul):
        //FeedMill
        case (18062285989211648686ul):
        //Milling
        case (2988354002298813950ul):
            //processing branch
            if (get_skill_status(db, "Processing") != 0) {
                return NULL;
            }
            else {
                return "Dependancy needed: Processing";
            }
        break;
        //Farming
        case (11735882624484804429ul):
        //LiveStock
        case (8725773883252304405ul):
        //Processing
        case (3966044600816150607ul):
            return NULL;
        break;
        default:
            //nop
            return "Not found";
        break;
    }
}

static inline uint64_t hash(const char* key) {
    //see https://stackoverflow.com/a/57960443/14062392

    uint64_t h = 525201411107845655ull;

    for (;*key; key++) {
        h ^= *key;
        h *= 0x5bd1e9955bd1e995;
        h ^= h >> 47;
    }
    
    return h;
}
