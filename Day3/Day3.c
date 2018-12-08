#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <bsd/stdlib.h>

#include "file.h"
#include "utils.h"

typedef struct claim
{
    uint32_t id;
    int from_left;
    int from_top;
    int width;
    int height;
} claim_t;

bool parse_claim(line_t *line)
{
    char *s = line_string(line);
    claim_t *c = calloc(1, sizeof(claim_t));
    sscanf(s, "#%u @ %u,%u: %ux%u", &c->id, &c->from_left, &c->from_top, &c->width, &c->height);
    line->extra = c;
    return true;
}

void free_claim(line_t *line)
{
    free(line->extra);
}

typedef struct fabric
{
    uint32_t width;
    uint32_t height;
    uint32_t **matrix;
} fabric_t;

#define FABRIC_SIDE_MIN (1000)
#define MAX(__a, __b) (((__a) > (__b)) ? (__a) : (__b)) 
#define MIN(__a, __b) (((__a) < (__b)) ? (__a) : (__b)) 

#define UNIT_EMPTY 0
#define UNIT_TAKEN 1
#define UNIT_OVERLAP 2
// matrix[0]
// matric[1]

fabric_t *fabric_create(uint32_t min_x, uint32_t min_y)
{
    fabric_t *fabric = calloc(1, sizeof(fabric_t));
    fabric->width = MAX(min_x, FABRIC_SIDE_MIN);
    fabric->height = MAX(min_y, FABRIC_SIDE_MIN);
    fabric->matrix = calloc(fabric->height, sizeof(uint32_t *));
    for (uint32_t i = 0; i < fabric->height; ++i) {
        fabric->matrix[i] = calloc(fabric->width, sizeof(uint32_t));
    }

    return fabric;
}

void fabric_free(fabric_t *fabric)
{
    if (fabric && fabric->matrix) {
        for (uint32_t i = 0; i < fabric->height; ++i) {
            free(fabric->matrix[i]);
        }
        free(fabric->matrix);
        free(fabric);
    }
}

void fabric_claim_area(fabric_t *fabric, claim_t *claim)
{
    uint32_t y_end = claim->from_top + claim->height;
    uint32_t x_end = claim->from_left + claim->width;
    for (uint32_t y = claim->from_top; y < y_end; ++y) {
        for (uint32_t x = claim->from_left; x < x_end; ++x) {
            uint32_t *valuep = &fabric->matrix[y][x];
            switch (*valuep) {
                case UNIT_EMPTY:
                    *valuep = UNIT_TAKEN;
                    break;
                case UNIT_TAKEN:
                    *valuep = UNIT_OVERLAP;
                    break;
                default:
                    break;
            }
        }
    }
}

bool fabric_check_claim(fabric_t *fabric, claim_t *claim)
{
    uint32_t y_end = claim->from_top + claim->height;
    uint32_t x_end = claim->from_left + claim->width;
    for (uint32_t y = claim->from_top; y < y_end; ++y) {
        for (uint32_t x = claim->from_left; x < x_end; ++x) {
            uint32_t value = fabric->matrix[y][x];
            if (value == UNIT_OVERLAP) {
                return false;
            }
        }
    }
    return true;
}

uint32_t fabric_compute_overlap(fabric_t *fabric)
{
    uint32_t overlap = 0;
    for (uint32_t y = 0; y < fabric->height; ++y) {
        for (uint32_t x = 0; x < fabric->width; ++x) {
            if (fabric->matrix[y][x] == UNIT_OVERLAP) {
                overlap++;
            }
        }
    }
    return overlap;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: %s [input]\n", getprogname());
        return EXIT_FAILURE;
    }

    file_t *file = file_get_lines(argv[1], parse_claim, free_claim, NULL);
    DIE_IF((file == NULL), "Could not read lines from %s\n", argv[1]);

    int furthest_x = 0;
    int furthest_y = 0;
    line_t *line = NULL;
    size_t i = 0;
    file_for_each_line(file, line, i) {
        claim_t *claim = line_extra_data(line);
        int x = claim->from_left + claim->width;
        int y = claim->from_top + claim->height;
        if (x > furthest_x) {
            furthest_x = x;
        }

        if (y > furthest_y) {
            furthest_y = y;
        }
    }
    fabric_t *fabric = fabric_create(furthest_x, furthest_y);

    file_for_each_line(file, line, i) {
        claim_t *claim = line_extra_data(line);   
        fabric_claim_area(fabric, claim);
    }

    uint32_t overlap = fabric_compute_overlap(fabric);
    printf("Total Overlap: %u\n", overlap);

    file_for_each_line(file, line, i) {
        claim_t *claim = line_extra_data(line);   
        bool no_overlap = fabric_check_claim(fabric, claim);
        if (no_overlap) {
            printf("Claim %u doesn't have any overlap\n", claim->id);
            break;
        }
    }

    fabric_free(fabric);
    file_free(file);

    return 0;
}
