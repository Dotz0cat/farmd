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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include <libconfig.h>

typedef struct _config_settings config_settings;

struct _config_settings {
    int https_enable;

    int https_only;

    int https_port;

    int http_port;

    char* save_location;

    char* pub_key;

    char* priv_key;
};

typedef struct _pre_init_stuff pre_init_stuff;

struct _pre_init_stuff {
    char* save;

    char* config;

    char* xdg_config_home;

    char* home;

    config_settings* settings;
};

config_settings* config_parse(const char* config_location, const char* homedir, const char* xdg_config_home);

static inline int file_exsits(const char* file);
static inline int folder_exsits(const char* folder);
static void make_default_config(const char* output_file);

void free_pre_init_stuff(pre_init_stuff* pre_init);
void free_config_settings(config_settings* settings);

#endif /* CONFIG_H */
