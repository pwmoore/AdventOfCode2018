#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <bsd/stdlib.h>

#include "file.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: %s [input]\n", getprogname());
        return EXIT_FAILURE;
    }

    file_t *file = file_get_lines(argv[1], NULL, NULL);
    DIE_IF((file == NULL), "Could not read lines from %s\n", argv[1]);

    // FILL IN

    file_free(file);

    return 0;
}
