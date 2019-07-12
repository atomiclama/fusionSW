/*
 * log_file.c
 *
 *  Created on: 21 Dec 2015
 *      Author: gb7beda1
 */

#include <log_core.h>
#include <stdio.h>
#include <string.h>

static void fileLogger(const char * msg);

static FILE * logfile;

void log_fileInit() {

    logfile = fopen("/mmc/log.txt", "a+");

    if (logfile != NULL) {
        log_registerLogger(fileLogger);
    }
}

static void fileLogger(const char * msg) {
    if (logfile == NULL) {
        logfile = fopen("/mmc/log.txt", "a+");
    }
    fwrite(msg, 1, strlen(msg), logfile);

    fclose(logfile);
    logfile = NULL;
}
