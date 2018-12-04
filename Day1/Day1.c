#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <glib-2.0/gmodule.h>
#include <bsd/stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: %s [input file]\n", getprogname());
        exit(EXIT_FAILURE);
    }

    char *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    if (NULL == fp) {
        printf("Could not open %s: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    int linecount = 0;
    
    while ((read = getline(&line, &len, fp)) != -1) {
        linecount++;
    }

    rewind(fp);

    GHashTable *ht = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
    int *vals = calloc(linecount, sizeof(int));
    
    int sum = 0;
    int i = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        int val  = strtol(line, NULL, 0);
        sum += val;
        vals[i++] = val;
        if (g_hash_table_contains(ht, GINT_TO_POINTER(sum))) {
            printf("Found it: %d\n", sum);
            return EXIT_SUCCESS;
        }
        g_hash_table_add(ht, GINT_TO_POINTER(sum));
    }

    bool found = false;
    while (!found) {
        for (i = 0; i < linecount; ++i) {
            sum += vals[i];
            if (g_hash_table_contains(ht, GINT_TO_POINTER(sum))) {
                printf("Found it: %d\n", sum);
                return EXIT_SUCCESS;
            }
            g_hash_table_add(ht, GINT_TO_POINTER(sum));
        }   
    }

    return EXIT_SUCCESS;
}
