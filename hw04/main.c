#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "xparser.h"
#include "vector.h"

#define SINGLEPICK 2225445
#define MULTIPICK 3335445
#define FILEH 45646564
#define FILESTDIN 4564614
#define OUTFILEH 456465546
#define OUTFILESTDIN 45141414
#define ATTRVALPICK 32456477
#define ATTRPICK 324564546

void close_file_desc(FILE*file,FILE* file1){
    if (file != stdin){
        fclose(file);
    }
    if (file1 != stdout){
        fclose(file1);
    }
}
void xpath(struct node *node,struct vector *vector,char* array_arguments[], char* attributes[], char* values[], int element_number[], int layer,int all_layers );
void print_line(char *name, char *text, FILE * file,bool xml, char *attributes[], char *values[]){
    assert(name!= NULL);
    if(xml) {
        fprintf(file ,"<%s", name);

            for (int i = 0; i < MAXATSIZE; ++i) {
                if(attributes[i] != NULL){
                    assert(file != NULL);
                    fprintf(file, " %s=\"%s\"", attributes[i], values[i]);
                }
            }

        fprintf(file, ">\n");
        if (text != NULL) {
            fprintf(file, "%s\n", text);
        }
    } else{
        if (text != NULL) {
            fprintf(file, "%s\n", text);
        }
    }
}
void printing_to_terminal_node(struct node *node, FILE* file,bool xml){
    struct node *temp;
    if (node -> name != NULL){
        print_line(node ->name,node ->text, file, xml,node -> attributes, node ->values);
        if (node ->children != NULL) {
            for (int i = (int) node->children->size; i > 0; i--) {
                memcpy(&temp, vec_get(node->children, (node->children->size) - i), node->children->elem_size);
                printing_to_terminal_node(temp, file, xml);
            }
        }
        if(xml) {
            fprintf(file, "</%s>\n", node->name);
        }
        }

}
int printing_to_terminal(struct vector *vector, FILE* file, bool xml){
    struct node *node;
    struct node *temp;
    bool moreroots = false;
    size_t size = vector -> size;
    if (size > 1 && xml){
        moreroots = true;
        fprintf(file,"<result>\n");
    }
    for (size_t i = size; i > 0; i--) {
        //VECTOR
        memcpy(&node, vec_get(vector, (vector ->size) - i), vector ->elem_size);
        print_line(node ->name,node ->text, file, xml,node -> attributes, node ->values);
        //VECTOR
        if (node ->children != NULL) {
            //LAYER DOWN
            size_t size_b = node->children->size;
            for (size_t j = size_b; j > 0; j--) {
                memcpy(&temp, vec_get(node->children, (node->children->size) - j), node->children->elem_size);
                printing_to_terminal_node(temp, file, xml);
                //LAYER DOWN
            }
        }

        if(xml) {
            fprintf(file, "</%s>\n", node->name);
        }
    }
    if(moreroots){
        fprintf(file,"</result>\n" );
    }
    return 0;
}
void xpath_query_recursive(struct node *node,struct vector *vector,char* array_arguments[],
                           char* attributes[], char* values[], int element_number[], int layer,int all_layers ){
    struct node *temp;
    size_t elems = 0;
    size_t size = 0;
    if(node ->children != NULL) {
        elems = node->children->elem_size;
        size = node->children->size;
    }
    assert(vector != NULL);
    assert(node != NULL);

    if (layer + 1 != all_layers) {
        if (element_number[layer+1]>0){
            int count = 0;
            for (size_t i = size; i > 0; --i)  {
                memcpy(&temp, vec_get(node->children, size - i), elems);
                if(strcmp(temp->name, array_arguments[layer+1]) == 0) {
                    count++;
                    if (count == element_number[layer+1]){
                        xpath(temp, vector, array_arguments, attributes, values, element_number, layer + 1, all_layers);
                        if(layer+2 == all_layers){
                        vec_push_back(vector, &temp);}
                        return;
                    }
                }
            }
        }

        else {
            for (size_t i = size; i > 0; i--) {
                memcpy(&temp, vec_get(node->children, size - i), elems);
                xpath(temp, vector, array_arguments, attributes, values, element_number, layer + 1,all_layers);
            }
        }
    }   else
        vec_push_back(vector, &node);
}

int xpath_check(char* array_arguments[], char* attributes[], char* values[], const int element_number[], int layer){
    if(array_arguments[layer] && element_number[layer] == 0 && attributes[layer] == 0 && values[layer] == 0){
        return MULTIPICK;
    }
    else if(array_arguments[layer] && element_number[layer] > 0&& attributes[layer] == 0 && values[layer] == 0){
        return  SINGLEPICK;
    }
    else if(array_arguments[layer] && element_number[layer] == 0 && attributes[layer] != 0 && values[layer] == 0){
        return ATTRPICK;
    }
    else if(array_arguments[layer] && element_number[layer] == 0 && attributes[layer] != 0 && values[layer] != 0){
        return ATTRVALPICK;
    } else{
        fprintf(stderr, "bad usage");
        exit (-1);
    }
}
void xpath(struct node *node,struct vector *vector,char* array_arguments[], char* attributes[], char* values[], int element_number[], int layer,int all_layers ){
    struct node *temp;
    int type_of_query = xpath_check(array_arguments,attributes,values,element_number,layer);

    if (type_of_query == MULTIPICK){
        if (strcmp(node -> name, array_arguments[layer])==0 ) {
            xpath_query_recursive(node, vector, array_arguments,attributes,values, element_number, layer, all_layers);
        }
    }
    if (type_of_query == SINGLEPICK){
        if(node->children == NULL){
            return;}
        int count = 0;
        size_t size = node->children->size;


        for (size_t i = size; i > 0; --i)  {
            memcpy(&temp, vec_get(node->children, size - i), node ->children -> elem_size);
            if(strcmp(temp->name, array_arguments[layer+1]) == 0) {
                count++;
                if (count == element_number[layer+1]){
                    if(layer+2== all_layers){
                        vec_push_back(vector, &temp);
                        return;
                    }else {
                        xpath(temp, vector, array_arguments, attributes, values, element_number, layer + 1, all_layers);
                    }
                }

                }
            }
        }


    if (type_of_query == ATTRVALPICK){
        for (int x = 0; x < MAXATSIZE; ++x) {
            for (int j = 0; j < MAXATSIZE; ++j) {
            if(node->values[x] == NULL)
                continue;
            if(values[j] == NULL)
                    continue;
            if (strcmp(node->values[x], values[j]) == 0) {
                xpath_query_recursive(node, vector, array_arguments,attributes,values, element_number, layer, all_layers);
            }
            }
        }
    }
    if (type_of_query == ATTRPICK){
        for (int x = 0; x < MAXATSIZE; ++x) {
            for (int j = 0; j < MAXATSIZE; ++j) {
                if(attributes[j] == NULL)
                    continue;
                if (node->attributes[x] == NULL)
                    continue;
                if (strcmp(node->attributes[x], attributes[j]) == 0) {
                    xpath_query_recursive(node, vector, array_arguments, attributes, values, element_number, layer,
                                          all_layers);
                }
            }
        }
    }
}
int get_structure(struct node *node,struct vector *vector,char* array_arguments[],char* attributes[], char* values[] , int element_number[], int layers){
    int layer = 0;
    vector -> data = NULL;
    vector -> size = 0;
    vector -> capacity = 0;
    vector -> elem_size = 8;

   xpath(node, vector, array_arguments,attributes, values,element_number,layer, layers);
    return 0;
}
int xpath_formatting(struct node *node, char* query, struct vector* vector){
    int i = 0;
    char *ptr = strtok(query, "/");
    char *array_arguments[MAXATSIZE]= {0};
    char *attribues[MAXATSIZE]= {0};
    char *values[MAXATSIZE]= {0};
    int element_number[MAXATSIZE] = {0};
    int count = 0;
    int layers = 0;
    while (ptr != NULL)
    {   layers++;
        array_arguments[i++] = ptr;
        ptr = strtok (NULL, "/");
        count++;
    }

    for (int x = 0; x < count; ++x) {
        element_number[x] = 0;
        size_t length = strlen(array_arguments[x]);
        for (size_t j = 0; j < length; ++j) {
            if ((int) array_arguments[x][j] == (int) '['){
                if (isdigit(array_arguments[x][j + 1])) {
                    array_arguments[x][j] = '\0';
                    char number = array_arguments[x][j + 1];
                    element_number[x] = (int) number - 48;
                }
                if ((array_arguments[x][j + 1]) == '@') {
                    size_t index = j+2;
                    array_arguments[x][j] = '\0';
                    char str_atr[8048] = {0};
                    while(array_arguments[x][index] != ']'){
                        char ch = array_arguments[x][index];
                        strncat(str_atr, &ch, 1);
                        index++;
                    }
                    str_atr[index+1]='\0';
                    char * token = strtok(str_atr, "=");
                        attribues[x]=token;
                        token = strtok(NULL, " ");
                        values[x]=token;
                }
            }
        }
    }
    get_structure(node,vector,  array_arguments,attribues,values, element_number,  layers);
    return 0;
}



int argv_check(char * argument, const char * argument_next,  bool *xml_selected,  bool *text_selected) {
    if (argument[0] == '-') {
        if (strcmp(argument, "-i") == 0 || strcmp(argument, "--input") == 0) {
            if (argument_next[0] == '-' || !isalpha(argument_next[0])){
                return EXIT_FAILURE;
            }
            return FILEH;
        }
        else if (strcmp(argument, "-o") == 0 || strcmp(argument, "--output") == 0) {
            if (argument_next[0] == '-'|| !isalpha(argument_next[0])){
                return EXIT_FAILURE;
            }
            return OUTFILEH;
        }
        else if (strcmp(argument, "-t") == 0 || strcmp(argument, "--text") == 0) {
            *text_selected = true;
            if (*xml_selected && *text_selected) {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        } else if (strcmp(argument, "-x") == 0 || strcmp(argument, "--xml") == 0) {
            *xml_selected = true;
            if (*xml_selected && *text_selected) {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
    }
    return EXIT_FAILURE;
}
int query_check(const char *query){
    int i = 0;
    char c = 'c';
    int krtek = 0;
    while (c != '\0') {
        c = query[i];
        if(c == '[') {
            if (!isdigit(query[i+1]) && query[i+1]!='@'&& !isalpha(query[i+1]))
                return -1;
            if (query[i+1]=='0')
                return -1;
        }
        if(c == '/')
            if(!isalpha(query[i+1]))
                return -1;
        if(c == '"')
            krtek++;
        if(!isalpha(c) && c != '/' && c != '\0'&& c != ']'
        && c != '['&& c != '@'&& c != '='&& c != '"' &&c != '_'&& !isdigit(c)
        && c != '-'&& c != '.'&& c != ':'
        ){
            return -1;
        }
        if (c == '\0')
            if(!isalpha(query[i-1])&& query[i-1] !=']')
                return -1;
        i++;
    }
    if (krtek != 2 && krtek != 0)
        return -1;
    return 0;
}

int main(int argc, char **argv) {
    bool xml_selected = false;
    bool text_selected = false;
    FILE *file = stdin;
    FILE *file_out = stdout;
    int input_count =0;
    int output_count =0;
    for (int i = 1; i < argc-1; ++i) {
        if(argv_check( argv[i],argv[i+1],&xml_selected,&text_selected) !=0){
            if (argv_check( argv[i],argv[i+1],&xml_selected,&text_selected) == FILEH){
                file = fopen( argv[i+1],"r");
                i++;
                input_count++;
                if(file != NULL){
                    continue;
                }
            }
            if (argv_check( argv[i],argv[i+1],&xml_selected,&text_selected) == FILESTDIN){
                file = stdin;
                input_count++;
                continue;
            }
            if (argv_check( argv[i],argv[i+1],&xml_selected,&text_selected) == OUTFILEH){
                file_out = fopen( argv[i+1],"w+");
                i++;
                output_count++;
                if(file_out != NULL){
                    continue;
                }
            }
            if (argv_check( argv[i],argv[i+1],&xml_selected,&text_selected) == OUTFILESTDIN){
                output_count++;
                file_out = stdout;
                continue;
            }
            if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--text") == 0
            || strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--xml") == 0) {
                continue;
                }
            close_file_desc(file,file_out);
            fprintf(stderr, "bad usage");
            return -1;
        }
    }
    if(output_count >1 || input_count >1)
    {   close_file_desc(file,file_out);
        fprintf(stderr, "bad usage");
        return -1;
    }
   if (query_check(argv[argc-1]) != 0){
       close_file_desc(file,file_out);
        fprintf(stderr, "bad usage");
        return -1;
    }
    if (file_out == NULL || file == NULL){
        close_file_desc(file,file_out);
        fprintf(stderr, "bad usage");
        return -1;
    }
    if (text_selected == false && xml_selected == false){
        text_selected = true;
    }
    struct node *base_data = parse_xml(file);
    struct vector *result_vector = malloc(sizeof(struct vector));
    if(xpath_formatting(base_data, argv[argc-1],result_vector) != 0){
        close_file_desc(file,file_out);
        node_destroy(base_data);
        free(result_vector);
        return -1;
    }
    if(printing_to_terminal(result_vector,file_out,xml_selected) != 0){
        close_file_desc(file,file_out);
        free(result_vector);
        return -1;
    }
    node_destroy(base_data);
    vec_destroy(result_vector,NULL);

    close_file_desc(file,file_out);
    return 0;
}
