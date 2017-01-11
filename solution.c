#include "index.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* s_exit_command = "exit";
static const char* s_test_command = "--test";

static void read_input(Index_t* index, char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (!fp)
        return;

    timeval start = utils_get_time();

    char* line = NULL;
    size_t size = 0;
    size_t length = 0;
    size_t lines = 0;
    while ((size = getline(&line, &length, fp)) != -1) {
        size = utils_trim_line(line, size);
        if (size > 0) {
            bool result = index_find(index, line, size);
            if (!result) {
                printf("- %zu) %s\n", lines, line);
            }
        }
        lines++;
    }
    double duration = utils_get_time_duration_ms(start);
    printf("\n  Find done! %zu lines ~ %.4f (s), %.4f(ms) per line\n\n", lines, duration / 1000, (duration > 0 ? duration / lines : 0));
    if (line)
        free(line);
    fclose(fp);
}

static void read_cin(Index_t* index)
{
    size_t exit_length = strlen(s_exit_command);
    size_t line_size = max(index->max_string_size + 1, exit_length);

    char* line = (char*) malloc(line_size);
    ssize_t size = 0;

    while ((size = read(STDIN_FILENO, line, line_size)) > 0) {
        if (strncmp(line, s_exit_command, exit_length) == 0)
            break;

        size = utils_trim_line(line, size);
        bool result = index_find(index, line, size);
        printf("%s\n", result?"YES":"NO");
    }
    free(line);
}

int32_t main(int32_t argn, char** argv)
{
    if (argn < 2)
        return 0;

    bool use_debug = false;
    if (argn == 3 && strncmp(argv[2], s_test_command, strlen(s_test_command)) == 0)
        use_debug = true;

    char* filename = argv[1];
    Index_t* index = index_create_from_file(filename, use_debug);
    if (index) {
        if (use_debug)
            read_input(index, filename);
        else
            read_cin(index);

        index_destroy(index);
    }
    return 0;
}

