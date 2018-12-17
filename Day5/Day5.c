#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <bsd/stdlib.h>

#include "file.h"
#include "utils.h"

#define TRIGGER ((int)0x20)

#define SHOULD_REACT(__a, __b) (abs(((int)__a) - ((int)__b)) == TRIGGER)
#define DESTROY(__buf, __a, __b, __cnt) do { __buf[__a] = '\0'; __buf[__b] = '\0'; if (__a == 0) { __a += 2; __b += 2; } else { __a -= 1; __b += 1; } __cnt -= 2; } while (0)

typedef struct polymer {
    char *units;
    size_t total_size;
    size_t current_size;
    size_t start;
    size_t current;
    size_t next;
} polymer_t;

polymer_t *polymer_create(char *buf, size_t size)
{
    polymer_t *polymer = calloc(1, sizeof(polymer_t));
    polymer->units = calloc(1, size);
    memcpy(polymer->units, buf, size);
    
    if (polymer->units[size - 1] == '\n') {
        polymer->units[size - 1] = '\0';
        size -= 1;
    }
    polymer->current_size = polymer->total_size = size;
    polymer->current = 0;
    polymer->start = 0;
    polymer->next = 1;
    return polymer;
}

void polymer_free(polymer_t *polymer)
{
    if (polymer) {
        free(polymer->units);
        free(polymer);
    }
}

void polymer_print(polymer_t *polymer)
{
    char buf[polymer->current_size + 1];
    memset(buf, 0, polymer->current_size + 1);
    for (size_t i = 0, j = 0; i < polymer->total_size; ++i) {
        if (polymer->units[i] != '\0') {
            buf[j++] = polymer->units[i];
        }
    }
    printf("%s (C: %zu, N: %zu, Current: %zu, Total: %zu\n", buf, polymer->current, polymer->next, polymer->current_size, polymer->total_size);
}

static inline bool polymer_test(polymer_t *polymer)
{
    char a = polymer->units[polymer->current];
    char b = polymer->units[polymer->next];
    if (SHOULD_REACT(a, b)) {
        return true;
    } else {
        return false;
    }
}

static inline bool polymer_done_processing(polymer_t *polymer)
{
    return (polymer->next == (polymer->total_size - 1));
}

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

static inline void polymer_update(polymer_t *polymer)
{
    char a = polymer->units[polymer->current];
    char b = polymer->units[polymer->next];

    // Case 1: We just destroyed a pair.
    if (a == '\0' && b == '\0') {
        // Special case: we destroy the first two units
        if (unlikely(polymer->current == polymer->start)) {
            // Skip over the destroyed units and set start to current
            polymer->current += 2;
            polymer->next += 2;
            polymer->start = polymer->current;
        } else {
            while (polymer->units[polymer->current] == '\0') {
                polymer->current -= 1;
            }
            polymer->next += 1;
        }

        // Update size
        polymer->current_size -= 2;
    } else {
        // Case 2: We didn't destroy anything. Set current equal to next
        // and next is incremented by one
        polymer->current = polymer->next;
        polymer->next += 1;
    }
}

static inline void polymer_remove(polymer_t *polymer, char c)
{
    for (size_t i = 0; i < polymer->total_size; ++i) {
        if (polymer->units[i] == c || polymer->units[i] == (c + TRIGGER)) {
            polymer->units[i] = '\0';
            polymer->current_size -= 1;
        }
    }
    char *newbuf = calloc(1, polymer->current_size + 1);
    memset(newbuf, 0, polymer->current_size + 1);
    for (size_t i = 0, j = 0; i < polymer->total_size; ++i) {
        if (polymer->units[i] != '\0') {
            newbuf[j++] = polymer->units[i];
        }
    }
    polymer->total_size = polymer->current_size;
    free(polymer->units);
    polymer->units = newbuf;
    //polymer_print(polymer);
}

struct args {
    char c;
    char *buf;
    size_t size;
};

void polymer_destroy(polymer_t *polymer)
{
#if 0
    char a = polymer->units[polymer->current];
    char b = polymer->units[polymer->next];
    printf("Destroyed %c%c\n", a, b);
#endif
    polymer->units[polymer->current] = '\0';
    polymer->units[polymer->next] = '\0';
}

void *find_size_thread(void *args)
{
    struct args *a = args;
    polymer_t *p = polymer_create(a->buf, a->size);
    polymer_remove(p, a->c);
    while (!polymer_done_processing(p)) {
        if (polymer_test(p)) {
            polymer_destroy(p);
        }
        polymer_update(p);
    }
    
    return (void *)p->current_size;
}

int main(int argc, char *argv[])
{
    size_t polymer_length = 0;
    polymer_t *polymer = NULL;
    if (argc < 2) {
        printf("usage: %s [input]\n", getprogname());
        return EXIT_FAILURE;
    }

    file_t *file = file_open(argv[1]);
    DIE_IF((file == NULL), "Could not read input %s\n", argv[1]);

    polymer_length = file_size(file);
    char *buf = file_contents(file);
    polymer = polymer_create(buf, polymer_length);

    while (!polymer_done_processing(polymer)) {
        if (polymer_test(polymer)) {
            polymer_destroy(polymer);
        }
        polymer_update(polymer);
        //polymer_print(polymer);
    }

    printf("The answer is %zu\n", polymer->current_size);
    size_t best_size = polymer->current_size;
    polymer_free(polymer);

#if 1
    pthread_t threads[26];
    struct args *args = calloc(26, sizeof(struct args));
    for (char c = 'A'; c <= 'Z'; ++c) {
        int i = c - 'A';
        args[i].c = c;
        args[i].buf = buf;
        args[i].size = polymer_length;
        pthread_create(&threads[i], NULL, find_size_thread, &args[i]);
    }
    for (char c = 'A'; c <= 'Z'; ++c) {
        int i = c - 'A';
        size_t ret = 0;
        pthread_join(threads[i], (void **)&ret);
        if (ret < best_size) {
            best_size = ret;
        }
    }

    printf("The new best size is %zu\n", best_size);
#endif
    file_free(file);

    return 0;
}
