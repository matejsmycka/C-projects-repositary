#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define DESTRUCTOR(X) ((void (*)(void *)) (X))



struct vector
{
    char *data;

    size_t size;
    size_t capacity;

    size_t elem_size;
};

typedef void (*destroy_function) (void *);


struct vector *vec_create(size_t elem_size);

void vec_destroy(struct vector *vec, destroy_function destructor);

void *vec_get(struct vector *vec, size_t pos);

bool vec_push_back(struct vector *vec, void *elem);

void vec_pop_back(struct vector *vec, void *dest);

size_t vec_size(const struct vector *vec);

bool vec_empty(const struct vector *vec);


#endif /* VECTOR_H */
