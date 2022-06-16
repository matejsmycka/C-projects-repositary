#include "managed_string.h"
#include "parser.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


static size_t min(size_t a, size_t b)
{
    return a < b ? a : b;
}

static size_t max(size_t a, size_t b)
{
    return a > b ? a : b;
}

/*****************************************************************************
 *  PARSING (RING) BUFFER
 *****************************************************************************/

bool buffer_fill_at(struct parsing_state *state, size_t start, size_t size)
{
    assert(state != NULL);

    if (size == 0) {
        return true;
    }

    void *dest = state->buffer.buffer + start;
    const long written = state->gen.fill(state->gen.data, dest, size);

    if (written < 0) {
        other_error(state, "Problem with reading input");
    } else if (written == 0) {
        state->gen.empty = true;
    } else {
        state->buffer.size += written;
    }

    return written > 0;
}

bool buffer_fill(struct parsing_state *state)
{
    assert(state != NULL);

    struct parsing_buffer *buffer = &state->buffer;

    size_t size = BUFFER_SIZE - max(buffer->pos + 1, buffer->history);
    if (!buffer_fill_at(state, buffer->pos + 1, size)) {
        return false;
    }

    size = BUFFER_SIZE - size - buffer->history;
    return buffer_fill_at(state, 0, size);
}

int buffer_peek(struct parsing_buffer *buffer)
{
    assert(buffer != NULL);

    return buffer->buffer[(buffer->pos + 1) % BUFFER_SIZE];
}

void buffer_shift_right(struct parsing_buffer *buffer)
{
    assert(buffer != NULL);

    buffer->pos = (buffer->pos + 1) % BUFFER_SIZE;
    --buffer->size;
    buffer->history = min(buffer->history + 1, LOOKAHEAD);
}

void buffer_shift_left(struct parsing_buffer *buffer)
{
    assert(buffer != NULL);
    assert(buffer->history > 0);

    buffer->pos = (buffer->pos + BUFFER_SIZE - 1) % BUFFER_SIZE;
    ++buffer->size;
    --buffer->history;
}


/*****************************************************************************
 *  GENERATORS
 *****************************************************************************/

long file_fill(void *data, void *dest, size_t size)
{
    assert(data != NULL);
    assert(dest != NULL);

    struct file_generator *file_gen = data;

    const size_t fread_ret = fread(dest, 1, size, file_gen->file);
    if (fread_ret == 0 && ferror(file_gen->file)) {
        return -1;
    }

    return fread_ret;
}

long str_fill(void *data, void *dest, size_t size)
{
    assert(data != NULL);
    assert(dest != NULL);

    struct str_generator *str_gen = data;

    const size_t amount = min(size, str_gen->len);
    memcpy(dest, str_gen->str, amount);
    str_gen->str += amount;
    str_gen->len -= amount;

    return amount;
}


/*****************************************************************************
 *  ERROR HANDLING
 *****************************************************************************/

static void _error(struct parsing_state *state, enum parsing_error_code code,
                   const char *format, ...)
{
    assert(state != NULL);
    assert(format != NULL);

    /* don't overwrite the previous error */
    if (state->error.code == PARSING_SUCCESS) {
        state->error.code = code;

        va_list args;
        va_start(args, format);
        vsnprintf(state->error.message, ERROR_MESSAGE_SIZE - 1, format, args);
        va_end(args);
    }
}

void parsing_error(struct parsing_state *state, const char *what)
{
    const char c = peek_char(state);
    _error(state, PARSING_ERROR, "Expecting '%s' but got '%s'",
           what, c == -1 ? "EOF" : CHAR_TO_STR(c));
}

void alloc_error(struct parsing_state *state, const char *what)
{
    _error(state, ALLOC_ERROR, "Not enough memory for '%s'", what);
}

void other_error(struct parsing_state *state, const char *what)
{
    _error(state, OTHER_ERROR, "%s", what);
}

void print_error(struct parsing_state *state, FILE *output)
{
    assert(state != NULL);
    assert(output != NULL);

    fprintf(output, "%s on line %ld, column %ld.\n",
            state->error.message, state->line, state->column);
}


/*****************************************************************************
 *  PARSING
 *****************************************************************************/

struct parsing_state parsing_state_init(void *data, fill_function function)
{
    assert(BUFFER_SIZE > LOOKAHEAD);

    return (struct parsing_state) {
        .buffer     = {
            .buffer     = { 0 },
            .history    = 0,
            .pos        = BUFFER_SIZE - 1,
            .size       = 0,
        },
        .line       = 1,
        .column     = 1,
        .gen        = {
            .fill       = function,
            .data       = data,
            .empty      = false,
        },
        .error      = {
            .code = PARSING_SUCCESS,
            .message    = { 0 },
            .line       = 0,
            .column     = 0,
        },
    };
}


int peek_char(struct parsing_state *state)
{
    assert(state != NULL);

    if (state->gen.empty) {
        return EOF;
    }

    if (state->buffer.size == 0 && !buffer_fill(state)) {
        return EOF;
    }

    return buffer_peek(&state->buffer);
}

int next_char(struct parsing_state *state)
{
    assert(state != NULL);

    const char next = peek_char(state);
    buffer_shift_right(&state->buffer);

    ++state->column;

    if (next == '\n') {
        state->column = 1;
        ++state->line;
    }

    return next;
}

bool accept_char(struct parsing_state *state, char expected)
{
    assert(state != NULL);

    bool accepted = peek_char(state) == expected;
    if (accepted) {
        (void) next_char(state);
    }

    return accepted;
}

bool pattern_char(struct parsing_state *state, char expected)
{
    assert(state != NULL);

    const bool is_correct = accept_char(state, expected);
    if (!is_correct) {
        parsing_error(state, CHAR_TO_STR(expected));
    }

    return is_correct;
}

bool pattern_str(struct parsing_state *state, const char *expected)
{
    assert(state != NULL);
    assert(expected != NULL);

    for (; *expected != '\0'; ++expected) {
        if (!pattern_char(state, *expected)) {
            return false;
        }
    }

    return true;
}

bool read_spaces(struct parsing_state *state, size_t minimum)
{
    assert(state != NULL);

    size_t count = 0;
    for (; isspace(peek_char(state)); ++count) {
        next_char(state);
    }

    const bool enough_spaces = count >= minimum;
    if (!enough_spaces) {
        parsing_error(state, "whitespace");
    }

    return enough_spaces;
}

void return_char(struct parsing_state *state)
{
    buffer_shift_left(&state->buffer);
}

/*****************************************************************************
 *  TESTING
 *****************************************************************************/

void test_str_generator(void)
{
    const char *string = "ahas\n\ndasd \na  a\ns sad  ";
    struct str_generator s_data = { string, strlen(string) };

    struct parsing_state ps = parsing_state_init(&s_data, str_fill);

    for (const char *curr = string; *curr; ++curr) {
        assert(*curr == next_char(&ps));
    }

    for (int i = 0; i < 10; ++i) {
        assert(next_char(&ps) == EOF);
    }
}


void test_file_generator(void)
{
    /* create input file */
    const char *filename = "_test_file.txt";
    const char *test_content = "ahas\n\ndasd \na  a\ns sad  ";

    FILE *file = fopen(filename, "w");
    assert(file != NULL);

    size_t length = strlen(test_content);
    size_t written = fwrite(test_content, 1, length, file);
    assert(written == length);
    fclose(file);

    /* an actual test */
    file = fopen(filename, "r");
    assert(file != NULL);

    struct file_generator f_data = { file };

    struct parsing_state ps = parsing_state_init(&f_data, file_fill);

    for (const char *curr = test_content; *curr; ++curr) {
        assert(*curr == next_char(&ps));
    }

    for (int i = 0; i < 10; ++i) {
        assert(next_char(&ps) == EOF);
    }

    assert(remove(filename) == 0);
}
