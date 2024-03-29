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

#include "config_private.h"

config_settings *config_parse(const char *config_location, const char *homedir, const char *xdg_config_home) {
    config_settings *settings = malloc(sizeof(config_settings));

    int http_port;
    int https_port;
    int https_enable;
    int https_only;
    const char *pub_key_location;
    const char *priv_key_location;
    const char *save_location;

    if (file_exsits(config_location) != 0) {
        char *xdg_config_home_folder;

        if (xdg_config_home != NULL && strcmp(xdg_config_home, "") != 0) {
            xdg_config_home_folder = strdup(xdg_config_home);
        }
        else {
            //make default path
            int char_count = snprintf(NULL, 0, "%s%s", homedir, "/.config");
            if (char_count <= 0) {
                //tough luck
                abort();
            }
            char *config_folder = malloc(char_count + 1U);

            if (config_folder == NULL) {
                //tough luck
                abort();
            }

            snprintf(config_folder, char_count + 1U, "%s%s", homedir, "/.config");

            xdg_config_home_folder = config_folder;
        }

        if (folder_exsits(xdg_config_home_folder) != 0) {
            if (mkdir(xdg_config_home_folder, 0777) == -1) {
                abort();
            }
        }

        int char_count = snprintf(NULL, 0, "%s%s", xdg_config_home_folder, "/farmd");
        if (char_count <= 0) {
            abort();
        }
        
        char *farmd_config_folder = malloc(char_count + 1U);

        if (farmd_config_folder == NULL) {
            abort();
        }

        snprintf(farmd_config_folder, char_count + 1U, "%s%s", xdg_config_home_folder, "/farmd");

        if (folder_exsits(farmd_config_folder) != 0) {
            if (mkdir(farmd_config_folder, 0777) == -1) {
                abort();
            }
        }

        free(xdg_config_home_folder);
        free(farmd_config_folder);

        make_default_config(config_location);
    }

    config_t cfg;

    config_init(&cfg);

    if (config_read_file(&cfg, config_location) != CONFIG_TRUE) {
        free(settings);
        config_destroy(&cfg);
        return NULL;
    }

    if (config_lookup_bool(&cfg, "enable_https", &https_enable)) {
        settings->https_enable = https_enable;
    }
    else {
        settings->https_enable = 0;
    }

    if (https_enable == CONFIG_TRUE) {
        if (config_lookup_bool(&cfg, "https_only", &https_only)) {
            settings->https_only = https_only;
        }
        else {
            settings->https_only = CONFIG_FALSE;
        }

        if (config_lookup_int(&cfg, "https_port", &https_port)) {
            settings->https_port = https_port;
        }
        else {
            //something failed with the port
            //so set it manually or it won't work
            settings->https_port = 8443;
        }

        if (config_lookup_string(&cfg, "key_location", &pub_key_location)) {
            settings->pub_key = strdup(pub_key_location);
        }
        else {
            settings->pub_key = NULL;
        }

        if (config_lookup_string(&cfg, "priv_key_location", &priv_key_location)) {
            settings->priv_key = strdup(priv_key_location);
        }
        else {
            settings->priv_key = NULL;
        }
    }
    else {
        settings->https_only = 0;
        settings->pub_key = NULL;
        settings->priv_key = NULL;
    }

    if (settings->https_only == CONFIG_FALSE) {
        if (config_lookup_int(&cfg, "http_port", &http_port)) {
            settings->http_port = http_port;
        }
        else {
            settings->http_port = 8080;
        }
    }

    if (config_lookup_string(&cfg, "save_location", &save_location)) {
        settings->save_location = strdup(save_location);
    }
    else {
        settings->save_location = NULL;
    }

    config_destroy(&cfg);

    return settings;
}

//0 if exsits 1 if not
static inline int file_exsits(const char *file) {
    FILE *fp;

    if ((fp = fopen(file, "r"))) {
        fclose(fp);
        return 0;
    }
    else {
        return 1;
    }
}

static inline int folder_exsits(const char *folder) {
    DIR *dir;

    dir = opendir(folder);

    if (dir) {
        closedir(dir);
        return 0;
    }
    else {
        return 1;
    }
}

static void make_default_config(const char *output_file) {
    config_t cfg;

    config_init(&cfg);

    config_setting_t *root;

    root = config_root_setting(&cfg);

    config_setting_t *https_enable;

    https_enable = config_setting_add(root, "enable_https", CONFIG_TYPE_BOOL);
    config_setting_set_bool(https_enable, CONFIG_FALSE);

    config_setting_t *port;

    port = config_setting_add(root, "port", CONFIG_TYPE_INT);
    config_setting_set_int(port, 8080);

    if (config_write_file(&cfg, output_file) != CONFIG_TRUE) {
        //if this fails I cannot help you
        abort();
    }

    config_destroy(&cfg);
}

void free_pre_init_stuff(pre_init_stuff *pre_init) {
    if (pre_init != NULL) {
        if (pre_init->save != NULL) {
            free(pre_init->save);
        }

        if (pre_init->config != NULL) {
            free(pre_init->config);
        }

        if (pre_init->xdg_config_home != NULL) {
            free(pre_init->xdg_config_home);
        }

        if (pre_init->home != NULL) {
            free(pre_init->home);
        }

        if (pre_init->settings != NULL) {
            free_config_settings(pre_init->settings);
        }

        free(pre_init);
    }
}

void free_config_settings(config_settings *settings) {
    if (settings != NULL) {
        if (settings->save_location != NULL) {
            free(settings->save_location);
        }

        if (settings->pub_key != NULL) {
            free(settings->pub_key);
        }

        if (settings->priv_key != NULL) {
            free(settings->priv_key);
        }

        free(settings);
    }
}
