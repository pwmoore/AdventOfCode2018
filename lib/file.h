#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct line
{
    char *str;
    size_t len;
    void *extra;
} line_t;

typedef bool (*line_transform_t)(line_t *line);
typedef void (*line_data_free_t)(line_t *line);
typedef int (*line_sort_t)(const void *a, const void *b);

typedef struct file
{
    void *mapped_file;
    size_t nlines;
    size_t capacity;
    line_t **lines;
    line_sort_t sort_callback;
    line_data_free_t free_callback;
} file_t;


#define FILE_LINES_INITIAL_CAPACITY (1000)

#define file_for_each_line(__f, __l, __i) \
    for (__i = 0, __l = (line_t *)(((file_t *)__f)->lines[__i]); __i < file_line_count(__f); ++(__i), __l = (line_t *)(((file_t *)__f)->lines[__i])) \

#ifdef __cplusplus
extern "C" { 
#endif

static inline char *line_string(line_t *line)
{
    return line->str;
}

static inline size_t line_length(line_t *line)
{
    return line->len;
}

static inline void *line_extra_data(line_t *line)
{
    return line->extra;
}

static inline size_t file_line_count(file_t *file)
{
    return file->nlines;
}

static inline size_t file_total_capacity(file_t *file)
{
    return file->capacity;
}

void file_sort_lines(file_t *lines);
file_t *file_get_lines(const char *filename, line_transform_t transform_callback, line_data_free_t free_callback, line_sort_t sort_callback);
line_t *file_get_line(file_t *file, uint32_t lineno);
long *file_get_as_numbers(file_t *lines);
void file_free(file_t *file);

#ifdef __cplusplus
}
#endif

#endif
