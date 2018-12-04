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
    uint64_t bits[3];
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

int line_cmp(void *a, void *b) 
{
    line_t *l1 = *(line_t **)a;
    line_t *l2 = *(line_t **)b;
    return strcmp(l1->str, l2->str);
}

void file_sort_lines(file_lines_t *lines)
{
    qsort(lines->lines, lines->nlines, sizeof(lines->lines[0]), line_cmp);
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

file_lines_t *file_get_lines(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    VALIDATE_PTR_OR_RETURN(fp, NULL);

    file_lines_t *lines = calloc(1, sizeof(*lines));
    VALIDATE_PTR_OR_RETURN(lines, NULL);
    lines->capacity = FILE_LINES_INITIAL_CAPACITY;
    lines->lines = calloc(lines->capacity, sizeof(line_t *));

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
    unsigned long char_counts[26] = {0};
    for (size_t i = 0; i < line->len; ++i) {
        int index = line->str[i] - 'a';
        unsigned long oldcnt = char_counts[index];
        unsigned long newcnt = char_counts[index] + 1;
        if (oldcnt == 2 && newcnt > 2) {
            num2s--;
        } else if (oldcnt == 3 && newcnt > 3) {
            num3s--;
        } 
        
        if (newcnt == 2) {
            num2s++;
        } else if (newcnt == 3) {
            num3s++;
        }

        char_counts[index] = newcnt;
    }

    if (num2s > 0) {
        *counts2 = true;
    }

    if (num3s > 0) {
        *counts3 = true;
    }
}

int hamming_distance(line_t *l1, line_t *l2, char *outstr)
{
    int hd = 0;
    int out_index = 0;
    for (size_t i = 0; i < l1->len; ++i) {
        if (l1->str[i] != l2->str[i]) {
            hd++;
        } else {
            outstr[out_index++] = l1->str[i];
        }
    }

    return hd;
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

    char outstr[lines->lines[0]->len];
    for (uint32_t i = 0; i < lines->nlines; ++i) {
        line_t *l1 = file_lines_get_line(lines, i);
        for (uint32_t j = 0; j < lines->nlines; ++j) {
            if (j != i) {
                line_t *l2 = file_lines_get_line(lines, j);
                memset(outstr, 0, lines->lines[0]->len);
                int hd = hamming_distance(l1, l2, outstr);
                if (hd == 1) {
                    printf("%s\n", outstr);
                    return 0;
                }
            }
        }
    }
}
