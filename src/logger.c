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
        perror("Could not open log file for writing");
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

void *log_pnrs() {    
    while(1) {
        char *path, *timestamp = get_timestamp();
        asprintf(&path, "/tmp/mininet-wifi/log/%s_pnrs.log", own_addr);
        FILE *log_file = fopen(path, "a");
    
        if (!log_file) {
            perror("Could not open log file for writing");
            exit(1);
        }
        fprintf(log_file, "[%s]\n", "START");


        size_t num_keys = 0;
        void **keys = NULL;
        char *key;
        size_t i = 0;

        keys = cfuhash_keys(pnrs, &num_keys, 1);

        fprintf(log_file, "%s\n{\n", timestamp);
        for (i = 0; i < num_keys; i++) {
            key = keys[i];
            fprintf(log_file, "\t%s -> ", key);

            node_t *current = cfuhash_get(pnrs, key);
            fprintf(log_file, "%s", "[");
            while (current != NULL) {
                fprintf(log_file, "%s%s", current->val, current->next == NULL ? "]" : ", ");
                current = current->next;
            }

            fprintf(log_file, "%s", "\n");
        }
        fprintf(log_file, "%s", "}\n");

        //sleep(1);

        fclose(log_file);
        free(path);
    }
}
