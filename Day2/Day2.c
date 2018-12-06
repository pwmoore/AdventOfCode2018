#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <bsd/stdlib.h>

#include "file.h"
#include "utils.h"

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

    file_t *file = file_get_lines(argv[1], NULL, NULL);
    DIE_IF((file == NULL), "Could not read lines from %s\n", argv[1]);

    uint64_t num2s = 0, num3s = 0;
    line_t *line = NULL;
    for (uint32_t i = 0; i < file_line_count(file); ++i) {
        line = file_get_line(file, i);
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

    size_t line_size = line_length(file_get_line(file, 0));
    char outstr[line_size];
    for (uint32_t i = 0; i < file_line_count(file); ++i) {
        line_t *l1 = file_get_line(file, i);
        for (uint32_t j = 0; j < file_line_count(file); ++j) {
            if (j != i) {
                line_t *l2 = file_get_line(file, j);
                memset(outstr, 0, line_size);
                int hd = hamming_distance(l1, l2, outstr);
                if (hd == 1) {
                    printf("%s\n", outstr);
                    goto out;
                    return 0;
                }
            }
        }
    }

out:
    file_free(file);
}
