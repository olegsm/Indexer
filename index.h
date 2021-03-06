#ifndef _INDEX_H
#define _INDEX_H

#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/time.h>

#define MAX(a,b) ({ a > b ? a : b; })

typedef struct timeval timeval;

typedef struct Index {
    size_t max_string_size;
    size_t symbols;
    size_t nodes;
    size_t words;

    void* head;
} Index_t;

timeval utils_get_time();
double utils_get_time_duration_ms(timeval start);
size_t utils_trim_line(char* line, size_t size);

Index_t* index_create();
Index_t* index_create_from_file(char* filename, bool dump);
void index_destroy(Index_t* index);

void index_insert(Index_t* index, const char* string, size_t size);
bool index_find(Index_t* index, const char* string, size_t size);

void index_dump(Index_t* index);

#endif