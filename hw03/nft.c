#include <stdlib.h>
#include <string.h>
#include "capture.h"


void destroy_all(struct capture_t *capture, struct capture_t *filtered, struct capture_t *filtered2){
    destroy_capture(filtered);
    destroy_capture(filtered2);
    destroy_capture(capture);
    free(capture);
    free(filtered);
    free(filtered2);
}
int main_error_check(int MaskTo, int MaskFrom,int argc){
    if(MaskTo > 32 || MaskFrom < 0 || MaskTo < 0){
        fprintf(stderr, "Failure - could not parse a mask\n");
        return EXIT_FAILURE;
    }
    if (argc != 5) {
        fprintf(stderr, "usage: ./nft <input_file> <from+mask> <to+mask> <statistic>\n");
        return EXIT_FAILURE;
    }
    return 0;
}
int filtering(struct capture_t *capture, struct capture_t *filtered,
        struct capture_t *filtered2, int MaskFrom,int MaskTo,
                uint8_t *UAddTo, uint8_t *UAddFrom){

    if(filter_from_mask(capture,filtered,UAddFrom,MaskFrom) != EXIT_SUCCESS){
        destroy_all(capture,filtered2,filtered);
        fprintf(stderr, "filter failure\n");
        return EXIT_FAILURE;
    }

    if(filter_to_mask(filtered,filtered2,UAddTo,MaskTo) != EXIT_SUCCESS){
        destroy_all(capture,filtered2,filtered);
        fprintf(stderr, "filter failure\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int printing(struct capture_t *capture, struct capture_t *filtered, struct capture_t *filtered2,char * argument){
    if (strcmp(argument, "flowstats") == 0 ){
        if(print_flow_stats(filtered2) != EXIT_SUCCESS){
            destroy_all(capture,filtered2,filtered);
            return EXIT_FAILURE;
        }
    }
    else if (strcmp(argument, "longestflow") == EXIT_SUCCESS){
        if(print_longest_flow(filtered2)!= EXIT_SUCCESS){
            destroy_all(capture,filtered2,filtered);
            return EXIT_FAILURE;
        }
    }
    else{
        fprintf(stderr, "Failure - invalid statistic name\n");
        destroy_all(capture,filtered2,filtered);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    int AddFrom[4];
    int AddTo[4];
    int MaskFrom = 0;
    int MaskTo = 0;
    uint8_t UAddFrom[4];
    uint8_t UAddTo[4];

    if(sscanf(argv[2], "%d.%d.%d.%d/%d",&AddFrom[0],&AddFrom[1],&AddFrom[2],&AddFrom[3], &MaskFrom) != 5) {
        fprintf(stderr, "bad usage\n");
        return EXIT_FAILURE;
    }

    if(sscanf(argv[3], "%d.%d.%d.%d/%d", &AddTo[0],&AddTo[1],&AddTo[2],&AddTo[3], &MaskTo) != 5) {
        fprintf(stderr, "bad usage\n");
        return EXIT_FAILURE;
    }

    if(main_error_check(MaskTo, MaskFrom, argc)!= EXIT_SUCCESS){
        return EXIT_FAILURE;
   }

    for (int i = 0; i < 4; ++i) {
        UAddFrom[i] =  AddFrom[i];
        UAddTo[i] = AddTo[i];
    }

    struct capture_t *filtered = malloc(sizeof(struct capture_t));
    struct capture_t *capture = malloc(sizeof(struct capture_t));
    struct capture_t *filtered2 = malloc(sizeof(struct capture_t));

    if(capture == NULL || filtered2 == NULL || filtered == NULL){
        destroy_all(capture,filtered2,filtered);
        fprintf(stderr, "memory allocation fail\n");
        return EXIT_FAILURE;
    }
    if(load_capture(capture, argv[1])){
        destroy_all(capture,filtered2,filtered);
        fprintf(stderr, "load capture failure\n");
        return EXIT_FAILURE;
    }

    if(filtering(capture,filtered,filtered2, MaskFrom, MaskTo, UAddTo, UAddFrom) != EXIT_SUCCESS){
        return EXIT_FAILURE;
    }

    if(printing(capture,filtered,filtered2, argv[4])!= EXIT_SUCCESS){
        return EXIT_FAILURE;
    }

    destroy_all(capture,filtered2,filtered);
    return EXIT_SUCCESS;
}
