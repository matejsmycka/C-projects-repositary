#ifndef MANAGED_STRING_H
#define MANAGED_STRING_H

#include <stdbool.h>
#include <stdlib.h>


typedef char mchar;

#define CHAR_TO_STR(X)     (char[]) { X, '\0' }

/**
 * @brief Create a managed string. It keeps size and capacity stored
 * at the beginning of allocated memory. The data is stored right after that.
 *
 * @param chars NULL-terminated string from which
 * the managed string is constructed
 * @return mchar* pointer to data
 */
mchar *str_create(const char *chars);

/**
 * @brief Destroy managed string. If it's NULL, no operation is performed.
 */
void str_destroy(mchar *str);

/**
 * @brief Return the length of managed string.
 */
size_t str_len(const mchar *str);

/**
 * @brief Clears the managed string. It will set the size to 0,
 * but the capacity will not change.
 */
void str_clear(mchar *str);

/**
 * @brief Add character `c` at the end of managed string
 *
 * @param str pointer to managed string
 * @return if the operation was successful
 */
bool str_add_char(mchar **str, char c);

/**
 * @brief Add NULL-terminated string `chars` at the end of managed string
 *
 * @param str pointer to managed string
 * @return if the operation was successful
 */
bool str_add_chars(mchar **str, const char *chars);

/**
 * @brief Inserts NULL-terminated string `chars` to managed string
 * at the position `index`
 *
 * @param str pointer to managed string
 * @return if the operation was successful
 */
bool str_insert(mchar **str, size_t index, const char *chars);


#endif /* MANAGED_STRING_H */
