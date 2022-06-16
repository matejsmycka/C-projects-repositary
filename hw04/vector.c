#include "vector.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


struct vector;

struct vector *vec_create(size_t elem_size)
{
    struct vector *vec = calloc(1, sizeof(struct vector));
    if (vec != NULL) {
        vec->elem_size = elem_size;
    }

    return vec;
}

void vec_destroy(struct vector *vec, destroy_function destructor)
{
    if (vec == NULL) {
        return;
    }

    if (destructor != NULL) {
        for (size_t i = 0; i < vec_size(vec); ++i) {
            destructor(vec_get(vec, i));
        }
    }

    free(vec->data);
    free(vec);
}

void *vec_get(struct vector *vec, size_t pos)
{
    assert(vec != NULL);

    return vec->data + pos * vec->elem_size;
}

static bool vec_realloc(struct vector *vec, size_t new_capacity)
{
    assert(vec != NULL);

    vec->capacity = new_capacity;

    void *new_data = realloc(vec->data, vec->capacity * vec->elem_size);
    if (new_data != NULL) {
        vec->data = new_data;
    }

    return vec->data != NULL;
}

static bool vec_realloc_double(struct vector *vec)
{
    assert(vec != NULL);

    const size_t new_capacity = vec->capacity == 0 ? 1 : vec->capacity * 2;
    return vec_realloc(vec, new_capacity);
}

bool vec_push_back(struct vector *vec, void *elem)
{
    assert(vec != NULL);

    if (vec->size == vec->capacity && !vec_realloc_double(vec)) {
        return false;
    }

    memcpy(vec_get(vec, vec->size), elem, vec->elem_size);
    ++vec->size;

    return true;
}

void vec_pop_back(struct vector *vec, void *dest)
{
    assert(vec != NULL);
    assert(dest != NULL);
    assert(vec->size != 0);

    --vec->size;

    if (dest != NULL) {
        memcpy(dest, vec_get(vec, vec->size), vec->elem_size);
    }
}

size_t vec_size(const struct vector *vec)
{
    assert(vec != NULL);

    return vec->size;
}

bool vec_empty(const struct vector *vec)
{
    assert(vec != NULL);

    return vec_size(vec) == 0u;
}
