#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define da_init(arr) \
    do { \
        (arr)->data = NULL; \
        (arr)->size = 0; \
        (arr)->capacity = 0; \
    } while(0)

#define da_free(arr) \
    do { \
        free((arr)->data); \
        (arr)->data = NULL; \
        (arr)->size = 0; \
        (arr)->capacity = 0; \
    } while(0)

#define da_reserve(arr, new_cap) \
    do { \
        if ((new_cap) > (arr)->capacity) { \
            void* new_data = realloc((arr)->data, sizeof(*(arr)->data) * (new_cap)); \
            if (!new_data) { \
                fprintf(stderr, "dynarray: allocation failed\n"); \
                exit(1); \
            } \
            (arr)->data = new_data; \
            (arr)->capacity = (new_cap); \
        } \
    } while(0)

#define da_push(arr, value) \
    do { \
        if ((arr)->size >= (arr)->capacity) { \
            size_t new_cap = ((arr)->capacity == 0) ? 8 : (arr)->capacity * 2; \
            da_reserve(arr, new_cap); \
        } \
        (arr)->data[(arr)->size++] = (value); \
    } while(0)

#define da_pop(arr) \
    ((arr)->data[--(arr)->size])

#define da_last(arr) \
    ((arr)->data[(arr)->size - 1])

#define da_clear(arr) \
    ((arr)->size = 0)

#define da_size(arr) \
    ((arr)->size)

#define da_capacity(arr) \
    ((arr)->capacity)

/* Declare a dynamic array type */
#define da_type(type) \
    struct { \
        type* data; \
        size_t size; \
        size_t capacity; \
    }

#endif