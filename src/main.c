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

#include "main.h"

int main(int argc, char** argv) {

    int opt = 0;

    char* config = NULL;
    char* save = NULL;

    while((opt = getopt(argc, argv, "vc:s:")) != -1) {
        switch(opt) {
            case 'v':
                printf("%s version: %s\r\n", argv[0], VERSION);
                return EXIT_SUCCESS;
                break;
            case 'c':
                config = strdup(optarg);
                break;
            case 's':
                save = strdup(optarg);
                break;
            default: /* ? */
                fprintf(stderr, "Usage: %s [-v] [-c config] [-s save]\r\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    loop_context* context;
    context = malloc(sizeof(loop_context));

    context->pre_init_info = pre_init(config, save);

    //init_daemon();
    openlog("farmd", LOG_PID, LOG_DAEMON);

    syslog(LOG_NOTICE, "log started");

    loop_run(context);

    //clean up
    free_pre_init_stuff(context->pre_init_info);

    free(context);

    closelog();

    return EXIT_SUCCESS;
}

static void init_daemon(void) {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    chdir("/");

    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--) {
        close(x);
    }

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

    openlog("farmd", LOG_PID, LOG_DAEMON);
}

static pre_init_stuff* pre_init(char* config, char* save) {
    pre_init_stuff* info = malloc(sizeof(pre_init_stuff));

    char* home = getenv("HOME");

    if (home == NULL) {
        uid_t uid = getuid();
        struct passwd* pw = getpwuid(uid);
        home = pw->pw_dir;
    }

    info->home = strdup(home);

    char* xdg_config_home = getenv("XDG_CONFIG_HOME");

    if (xdg_config_home != NULL) {
        info->xdg_config_home = strdup(xdg_config_home);
    }
    else {
        info->xdg_config_home = NULL;
    }
    

    if (config != NULL) {
        info->config = config;
    }
    else {
        if (info->xdg_config_home != NULL && strcmp(info->xdg_config_home, "") != 0) {
            int count = snprintf(NULL, 0, "%s%s", info->xdg_config_home, "/farmd/config");
            if (count <= 0) {
                abort();
            }

            info->config = malloc(count + 1U);

            if (info->config == NULL) {
                abort();
            }

            snprintf(info->config, count + 1U, "%s%s", info->xdg_config_home, "/farmd/config");
        }
        else {
            int count = snprintf(NULL, 0, "%s%s", info->home, "/.config/farmd/config");
            if (count <= 0) {
                abort();
            }

            info->config = malloc(count + 1U);

            if (info->config == NULL) {
                abort();
            }

            snprintf(info->config, count + 1U, "%s%s", home, "/.config/farmd/config");
        }
    }

    if (save != NULL) {
        info->save = save;
    }
    else {
        info->save = NULL;
    }

    info->settings = config_parse(info->config, info->home, info->xdg_config_home);

    return info;
}
