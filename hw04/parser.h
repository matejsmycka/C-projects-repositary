#ifndef PARSER_H
#define PARSER_H

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE         32768
// 33 KB
#define LOOKAHEAD           16
#define ERROR_MESSAGE_SIZE  256


/*****************************************************************************
 *  GENERATORS
 *****************************************************************************/

/**
 * @brief Function that fills buffer `dest` with `size` bytes
 *
 * @return -1: error
 *          n: number of read bytes
 */
typedef long (*fill_function)(void *data, void *dest, size_t size);

struct generator
{
    fill_function fill;
    void *data;
    bool empty;
};

struct file_generator
{
    FILE *file;
};

long file_fill(void *data, void *dest, size_t size);

struct str_generator
{
    const char *str;
    size_t len;
};

long str_fill(void *data, void *dest, size_t size);


/*****************************************************************************
 *  PARSING
 *****************************************************************************/

enum parsing_error_code {
    PARSING_SUCCESS,

    PARSING_ERROR,
    ALLOC_ERROR,
    OTHER_ERROR,
};

struct parsing_buffer {
    char buffer[BUFFER_SIZE];
    size_t pos;
    size_t size;
    size_t history;
};

struct parsing_error {
    enum parsing_error_code code;
    char message[ERROR_MESSAGE_SIZE];

    size_t line;
    size_t column;
};

struct parsing_state {
    struct parsing_buffer buffer;

    size_t line;
    size_t column;

    struct generator gen;

    struct parsing_error error;
};


struct parsing_state parsing_state_init(void *data, fill_function function);

int peek_char(struct parsing_state *state);

int next_char(struct parsing_state *state);

bool accept_char( struct parsing_state *state, char expected );

bool pattern_char(struct parsing_state *state, char expected);

bool pattern_str(struct parsing_state *state, const char *expected);

bool read_spaces( struct parsing_state *state, size_t minimum );

void return_char(struct parsing_state *state);


/*****************************************************************************
 *  ERROR HANDLING
 *****************************************************************************/

void parsing_error(struct parsing_state *state, const char *what);

void alloc_error(struct parsing_state *state, const char *what);

void other_error(struct parsing_state *state, const char *what);

void print_error(struct parsing_state *state, FILE *output);


/*****************************************************************************
 *  TESTS
 *****************************************************************************/

void test_str_generator(void);

void test_file_generator(void);


#endif /* PARSER_H */
