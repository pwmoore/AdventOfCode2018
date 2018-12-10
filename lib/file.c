#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "utils.h"
#include "file.h"

int line_cmp(const void *a, const void *b) 
{
    line_t *l1 = *(line_t **)a;
    line_t *l2 = *(line_t **)b;
    return strcmp(l1->str, l2->str);
}

void file_sort_lines(file_t *file)
{
    qsort(file->lines, file->nlines, sizeof(file->lines[0]), file->sort_callback);
}

line_t *file_next_line(FILE *fp) {
    ssize_t bytes_read = 0;
    char *str = NULL;
    size_t len = 0;
    bytes_read = getline(&str, &len, fp);
    if (bytes_read < 0) {
        return NULL;
    }

    len = strlen(str);
    if (str[len - 1] == '\n') {
        str[len - 1] = '\0';
        len -= 1;
    }

    line_t *line = calloc(1, sizeof(*line));
    line->str = str;
    line->len = len;
    return line;
}

void file_line_free(line_t *line)
{
    if (line) {
        free(line->str);
        free(line);
    }
}

file_t *file_get_lines(const char *filename, line_transform_t transform_callback, line_data_free_t free_callback, line_sort_t sort_callback)
{
    FILE *fp = fopen(filename, "r");
    VALIDATE_PTR_OR_RETURN(fp, NULL);

    file_t *file= calloc(1, sizeof(*file));
    VALIDATE_PTR_OR_RETURN(file, NULL);
    file->capacity = FILE_LINES_INITIAL_CAPACITY;
    file->lines = calloc(file->capacity, sizeof(line_t *));

    line_t *line = NULL;
    while ((line = file_next_line(fp)) != NULL) {
        if (file->nlines == (file->capacity)) {
            size_t new_capacity = 2 * file->capacity;
            file->lines = realloc(file->lines, (new_capacity * sizeof(line_t *)));
            file->capacity = new_capacity;
        }
         
        if (transform_callback) {
            transform_callback(line);
        }

        file->lines[file->nlines++] = line;
    }

    if (free_callback) {
        file->free_callback = free_callback;
    }

    if (sort_callback) {
        file->sort_callback = sort_callback;
    } else {
        file->sort_callback = line_cmp;
    }

    fclose(fp);
    return file;
}

void file_free(file_t *file)
{
    if (file) {
        for (size_t i = 0; i < file->nlines; ++i) {
            line_t *line = file_get_line(file, i);
            if (line) {
                if (file->free_callback) {
                    file->free_callback(line);
                }
                file_line_free(line);
            }
        }
        if (file->contents) {
            free(file->contents);
        }
        free(file->lines);
        free(file);
    }
}

line_t *file_get_line(file_t *file, uint32_t lineno)
{
    if (lineno >= file->nlines) {
        return NULL;
    } else {
        return file->lines[lineno];
    }
}

long *file_lines_get_as_numbers(file_t *file)
{
    long *vals = calloc(file->nlines, sizeof(long));
    for (size_t i = 0; i < file->nlines; ++i) {
        vals[i] = strtol(file->lines[i]->str, NULL, 0);
    }
    return vals;
}

int file_get_size(const char *filename, size_t *size)
{
    struct stat sb;
    if (stat(filename, &sb)) {
        return -1;
    }

    *size = sb.st_size;
    return 0;
}

file_t *file_open(const char *filename)
{
    file_t *file = calloc(1, sizeof(file_t));
    FILE *fp = NULL;
    int err = file_get_size(filename, &file->size);
    if (err) {
        goto out;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        err = -1;
        goto out;
    }

    file->contents = calloc(1, file->size);

    if (file->contents == NULL) {
        err = -1;
        goto out;
    }

    if (fread(file->contents, 1, file->size, fp) != file->size) {
        err = -1;
        goto out;
    }

    err = 0;

out:
    if (err) {
        file_free(file);
        file = NULL;
    }
    fclose(fp);

    return file;
}
