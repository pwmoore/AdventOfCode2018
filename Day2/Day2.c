#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <bsd/stdlib.h>

#define COUNTER_OCCURRENCES(__ctr) ((__ctr) & 127)
#define COUNTER_COUNTS(__ctr) ((__ctr) & 128)

typedef struct line
{
    char *str;
    size_t len;
} line_t;

typedef struct file_lines
{
    size_t nlines;
    size_t capacity;
    line_t **lines;
} file_lines_t;

#define FILE_LINES_INITIAL_CAPACITY (1000)

#define DIE_IF(__cond, __msg, ...) if (__cond) { fprintf(stderr, "[X] " __msg "\n", ## __VA_ARGS__); exit(EXIT_FAILURE); }
#define ERR(__msg, ...) do { fprintf(stderr, "[X] " __msg "\n", ## __VA_ARGS__); } while (0)

#define VALIDATE_PTR_OR_RETURN(__ptr, __retval) if (0 == (__ptr)) { ERR(# __ptr "is NULL!"); return (__retval); }

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

file_lines_t *file_get_lines(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    VALIDATE_PTR_OR_RETURN(fp, NULL);

    file_lines_t *lines = calloc(1, sizeof(*lines));
    VALIDATE_PTR_OR_RETURN(lines, NULL);
    lines->capacity = FILE_LINES_INITIAL_CAPACITY;
    lines->lines = calloc(lines->capacity, sizeof(line_t));

    line_t *line = NULL;
    while ((line = file_next_line(fp)) != NULL) {
        if (lines->nlines == lines->capacity) {
            size_t new_capacity = 2 * lines->capacity;
            lines->lines = realloc(lines->lines, new_capacity);
            lines->capacity = new_capacity;
        }
        lines->lines[lines->nlines++] = line;
    }

    fclose(fp);
    return lines;
}

line_t *file_lines_get_line(file_lines_t *lines, uint32_t lineno)
{
    if (lineno >= lines->nlines) {
        return NULL;
    } else {
        return lines->lines[lineno];
    }
}

long *file_lines_get_as_numbers(file_lines_t *lines)
{
    long *vals = calloc(lines->nlines, sizeof(long));
    for (size_t i = 0; i < lines->nlines; ++i) {
        vals[i] = strtol(lines->lines[i]->str, NULL, 0);
    }
    return vals;
}

void count_letters(line_t *line, bool *counts2, bool *counts3)
{
    unsigned long num2s = 0, num3s=0;
    unsigned long vals[26];
    for (size_t i = 0; i < line->len; ++i) {
        int index = line->str[i] - 'a';
        unsigned long oldval = vals[index];
        unsigned long newval = vals[index] + 1;
        if (oldval == 2 && newval > 2) {
            num2s--;
        } else if (oldval == 3 && newval > 3) {
            num3s--;
        } else if (newval == 2) {
            printf("2: %s: %c\n", line->str, line->str[i]);
            num2s++;
        } else if (newval == 3) {
            printf("3: %s: %c\n", line->str, line->str[i]);
            num3s++;
        }

        vals[index] = newval;
    }

    if (num2s > 0) {
        printf("%s has a letter that occurs twice\n", line->str);
        *counts2 = true;
    }

    if (num3s > 0) {
        printf("%s has a letter that occurs three times\n", line->str);
        *counts3 = true;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: %s [input]\n", getprogname());
        return EXIT_FAILURE;
    }

    file_lines_t *lines = file_get_lines(argv[1]);
    DIE_IF((lines == NULL), "Could not read lines from %s\n", argv[1]);

    uint64_t num2s = 0, num3s = 0;
    line_t *line = NULL;
    for (uint32_t i = 0; i < lines->nlines; ++i) {
        line = file_lines_get_line(lines, i);
        bool count2 = false, count3 = false;
        count_letters(line, &count2, &count3);
        if (count2) {
            num2s++;
        }

        if (count3) {
            num3s++;
        }
    }

    printf("result: (%lu * %lu) =  %lu\n", num2s, num3s, (num2s * num3s));

}
