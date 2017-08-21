#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//#include "common.h"

char *get_timestamp();

void log_msg(char *message, char *addr) {
    char *path, *timestamp = get_timestamp();
    asprintf(&path, "/tmp/mininet-wifi/log/%s.log", addr);
    FILE *log_file = fopen(path, "a");

    if (!log_file) {
        perror("Could not open log file for writing");sleep(10);
        exit(1);
    }

    fprintf(log_file, "%s %s", timestamp, message);
    //printf("%s", message);

    fclose(log_file);
    free(path);
}

char *get_timestamp() {
    time_t timer;
    char *buffer = malloc(26 * sizeof(char));
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "[%d-%m-%Y %H:%M:%S]", tm_info);

    return buffer;
}