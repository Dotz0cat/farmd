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

#include "main.h"

int main(int argc, char** argv) {

    int opt = 0;

    while((opt = getopt(argc, argv, "v")) != -1) {
        switch(opt) {
            case 'v':
                printf("%s version: %s\r\n", argv[0], VERSION);
                return EXIT_SUCCESS;
                break;
            default: /* ? */
                fprintf(stderr, "Usage: %s [-v]\r\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    //init_daemon();
    openlog("farmd", LOG_PID, LOG_DAEMON);

    syslog(LOG_NOTICE, "log started");

    loop_context* context;
    context = malloc(sizeof(loop_context));
    context->number = 4;

    loop_run(context);

    free(context);

    return EXIT_SUCCESS;
}

static void init_daemon() {
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
