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


#ifndef LIST_PRIVATE_H
#define LIST_PRIVATE_H

#include "list.h"

static fields_list *add_field_to_list(fields_list *prev, int field_number);
static fields_list *wind_fields_to_tail(fields_list *list);
static trees_list *add_tree_to_list(trees_list *prev, int tree_number);
static trees_list *wind_trees_to_tail(trees_list *head);

#endif /* LIST_PRIVATE_H */
