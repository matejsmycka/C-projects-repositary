#ifndef XPARSER_H
#define XPARSER_H

#include "managed_string.h"
#include "parser.h"

#include <stdio.h>
#define MAXATSIZE 10000

/*****************************************************************************
 *  DATA STRUCTURE
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *  S
 *****************************************************************************/

struct node {
    mchar *name;

    mchar *key;
    mchar *value;
    mchar *text;
    char *attributes[MAXATSIZE];
    char *values[MAXATSIZE];
    struct vector *children;
};

struct node *node_create(mchar *name, mchar *key, mchar *value,
                         mchar *text, struct vector *children);

void node_destroy(struct node *node);


void node_ptr_destroy(struct node **node);
/*****************************************************************************
 *  MISCELLANEOUS
 *****************************************************************************/

typedef int (*predicate) (int);

mchar *parse_string(struct parsing_state *state, predicate valid_chars);

mchar *parse_word(struct parsing_state *state, predicate start,
                  predicate rest, const char *error_message);


/*****************************************************************************
 *  PARSING
 *****************************************************************************/

mchar *parse_name(struct parsing_state *state);

mchar *parse_text(struct parsing_state *state, bool normalize);

bool parse_equals(struct parsing_state *state);

mchar *parse_value(struct parsing_state *state);

bool parse_attributes(struct parsing_state *state, mchar **key, mchar **value,char *attributes[],char *values[],size_t * attribute_length);

struct node *parse_xnode(struct parsing_state *state);

struct vector *parse_xnodes(struct parsing_state *state);

struct node *parse_root(struct parsing_state *state);

struct node *parse_xml(FILE *file);


#endif /* XPARSER_H */
