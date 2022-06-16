#include "managed_string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


#define HEADER_SIZE   (2 * sizeof(size_t))

#define STRING(X)     ((mchar *) (X + HEADER_SIZE))
#define RAW(X)        ((char *) (X - HEADER_SIZE))
#define LENGTH(X)     ((size_t *) (RAW(X)))[0]
#define CAPACITY(X)   ((size_t *) (RAW(X)))[1]


static size_t _calculate_capacity(const mchar *str, size_t length)
{
    size_t capacity = str == NULL ? HEADER_SIZE : CAPACITY(str);
    while (length > capacity) {
        capacity *= 2;
    }

    return capacity;
}

static mchar *_str_realloc(mchar *str, size_t length)
{
    const size_t new_length = length + HEADER_SIZE;
    const size_t new_capacity = _calculate_capacity(str, new_length);

    if (str == NULL || new_capacity > CAPACITY(str)) {
        char *raw_string = str == NULL ? NULL : RAW(str);
        mchar *new_string = realloc(raw_string, new_capacity);
        if (new_string == NULL) {
            return NULL;
        }

        str = STRING(new_string);
    }

    LENGTH(str) = new_length - HEADER_SIZE - 1;
    CAPACITY(str) = new_capacity;

    return str;
}

mchar *str_create(const char *chars)
{
    assert(chars != NULL);

    const size_t size = strlen(chars) + 1;
    mchar *str = _str_realloc(NULL, size);
    if (str != NULL) {
        memcpy(str, chars, size);
    }

    return str;
}

void str_destroy(mchar *str)
{
    if (str != NULL) {
        free(RAW(str));
    }
}

size_t str_len(const mchar *str)
{
    assert(str != NULL);

    return LENGTH(str);
}

bool str_add_chars(mchar **str, const char *chars)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(chars != NULL);

    return str_insert(str, str_len(*str), chars);
}

bool str_add_char(mchar **str, char c)
{
    assert(str != NULL);
    assert(*str != NULL);

    return str_add_chars(str, CHAR_TO_STR(c));
}

bool str_insert(mchar **str, size_t index, const char *chars)
{
    assert(str != NULL);
    assert(*str != NULL);
    assert(chars != NULL);

    const size_t size =  str_len(*str);
    assert(index <= size);

    const size_t chars_len = strlen(chars);
    const size_t new_length = size + chars_len + 1;

    mchar *new_str = _str_realloc(*str, new_length);
    if (new_str == NULL) {
        return false;
    }

    *str = new_str;

    char *dest = new_str + index;
    memmove(dest + chars_len, dest, size - index + 1);
    memcpy(dest, chars, chars_len);

    return true;
}

void str_clear(mchar *str)
{
    assert(str != NULL);

    str[0] = '\0';
    LENGTH(str) = 0;
}
