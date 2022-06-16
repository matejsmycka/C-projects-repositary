#include <stdlib.h>
#include "capture.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#define NONADRESS  300300300300

void load_capture_destroy(struct pcap_context *context, struct pcap_header_t *pcap_header){
    destroy_context(context);
    free(context);
    free(pcap_header);
}
int load_capture_check( const char *filename,struct capture_t *capture ,
        struct pcap_context *context, struct pcap_header_t *pcap_header){
    if (filename == NULL) {
        return EXIT_FAILURE;
    }
    if (capture == NULL) {
        return EXIT_FAILURE;
    }
    if (context == NULL || pcap_header == NULL){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int load_capture(struct capture_t *capture, const char *filename) {
    size_t NumberOfPackets = 10;
    struct pcap_header_t *pcap_header = malloc(sizeof(struct pcap_header_t));
    struct pcap_context *context = malloc(sizeof(struct pcap_context));

    if (load_capture_check(filename, capture, context, pcap_header)){
        return EXIT_FAILURE;
    }
    
    capture->PacketArray = malloc(10 * sizeof(struct packet_t));
    if (context == NULL){
        return EXIT_FAILURE;
    }
    if (init_context(context, filename) != PCAP_SUCCESS) {
        load_capture_destroy(context,pcap_header);
        return EXIT_FAILURE;
    }
    if (pcap_header == NULL) {
        load_capture_destroy(context,pcap_header);
        return EXIT_FAILURE;
    }
    if (load_header(context, pcap_header) != PCAP_SUCCESS) {
        load_capture_destroy(context,pcap_header);
        return EXIT_FAILURE;
    }
    capture->pcapHeader = pcap_header;
    capture->packet_count = 0;
    size_t index = 0;
    if ( capture->PacketArray == NULL) {
        load_capture_destroy(context,pcap_header);
        return EXIT_FAILURE;
    }
    while (true) {
        capture->PacketArray[index] = malloc(sizeof(struct packet_t));
        if (load_packet(context, capture->PacketArray[index]) != PCAP_SUCCESS) {
            //end of pcap
            if (load_packet(context, capture->PacketArray[index]) == PCAP_FILE_END) {
                destroy_context(context);
                free(capture->PacketArray[index]);
                free(context);
                return 0;
            }
            // load fail
            destroy_capture(capture);
            load_capture_destroy(context,pcap_header);
            free(capture->PacketArray[index]);
            free(capture);
            return EXIT_FAILURE;
        }
        // zvetseni pole
        if ((NumberOfPackets - 1) == index) {
            NumberOfPackets *= 2;
            capture->PacketArray = realloc(capture->PacketArray, NumberOfPackets * sizeof(struct packet_t));
        }
        capture->packet_count += 1;
        index++;

    }
}

void destroy_capture(struct capture_t *capture) {
    free(capture->pcapHeader);
    for (size_t i = 0; i < packet_count(capture); ++i) {
        destroy_packet(capture->PacketArray[i]);
        free(capture->PacketArray[i]);
    }
    free(capture->PacketArray);
}

const struct pcap_header_t *get_header(const struct capture_t *const capture) {
    return capture->pcapHeader;
}

struct packet_t *get_packet(
        const struct capture_t *const capture,
        size_t index) {
    if (packet_count(capture) == 0){
        return NULL;
    }
    if (index == packet_count(capture)){
        return NULL;
    }
    if (index > packet_count(capture)) {
        return NULL;
    }
    return capture->PacketArray[index];
}

size_t packet_count(const struct capture_t *const capture) {
    const size_t counter = capture->packet_count;
    return counter;
}

size_t data_transfered(const struct capture_t *const capture) {
    size_t count = packet_count(capture);
    size_t BitCount = 0;
    for (size_t i = 0; i < count; ++i) {
        BitCount += capture->PacketArray[i]->packet_header->orig_len;
    }
    return BitCount;
}
int error_check_mask(const struct capture_t *original,struct capture_t *filtered){
    if (original == NULL) {
        return EXIT_FAILURE;
    }
    if (filtered == NULL) {
        return EXIT_FAILURE;
    }
    return 0;
}
int filter_protocol(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t protocol) {
    filtered->pcapHeader = malloc(sizeof(struct pcap_header_t));
    if (error_check_mask(original,filtered) != 0){
        return EXIT_FAILURE;
    }
    if(memcpy(filtered->pcapHeader, original->pcapHeader, sizeof(struct packet_header_t)) == NULL){
        free(filtered->pcapHeader);
        return EXIT_FAILURE;
    }
    size_t NumberOfPackets = 10;
    size_t count = packet_count(original);
    int FilteredCount = 0;
    filtered->PacketArray = malloc(NumberOfPackets * sizeof(struct packet_t));
    if (filtered->pcapHeader == NULL){
        free(filtered->pcapHeader);
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < count; ++i) {

        if (original->PacketArray[i]->ip_header->protocol == protocol) {
            filtered->PacketArray[FilteredCount] = malloc(sizeof(struct packet_t));
            if (filtered->PacketArray[FilteredCount] == NULL) {
                return EXIT_FAILURE;
            }
            if (copy_packet(original->PacketArray[i], filtered->PacketArray[FilteredCount])) {
                free(filtered -> pcapHeader);
                free(filtered->PacketArray);
                return EXIT_FAILURE;
            }
            FilteredCount++;
        }
        if ((NumberOfPackets - 1) == i) {
            NumberOfPackets *= 2;
            filtered->PacketArray = realloc(filtered->PacketArray, NumberOfPackets * sizeof(struct packet_t));
        }
    }
    filtered->packet_count = FilteredCount;
    return 0;
}

int filter_larger_than(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint32_t size) {
    if (original == NULL) {
        return EXIT_FAILURE;
    }
    if (filtered == NULL) {
        return EXIT_FAILURE;
    }
    filtered->pcapHeader = malloc(sizeof(struct pcap_header_t));
    if (filtered->pcapHeader == NULL) {
        return EXIT_FAILURE;
    }
    if(memcpy(filtered->pcapHeader, original->pcapHeader, sizeof(struct packet_header_t))== NULL){
        free(filtered->pcapHeader);
        return EXIT_FAILURE;

    }
    size_t NumberOfPackets = 10;
    size_t count = packet_count(original);
    size_t FilteredCount = 0;
    filtered->PacketArray = malloc(NumberOfPackets * sizeof(struct packet_t));
    if (filtered->PacketArray == NULL){
        free(filtered->pcapHeader);
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < count; ++i) {

        if (original->PacketArray[i]->packet_header->orig_len > size) {
            filtered->PacketArray[FilteredCount] = malloc(sizeof(struct packet_t));
            if (filtered->PacketArray[FilteredCount] == NULL) {
                return EXIT_FAILURE;
            }
            if (copy_packet(original->PacketArray[i], filtered->PacketArray[FilteredCount])) {
                free(filtered -> pcapHeader);
                free(filtered->PacketArray);
                return EXIT_FAILURE;
            }
            FilteredCount++;
            if ((NumberOfPackets - 1) == FilteredCount) {
                NumberOfPackets *= 2;
                filtered->PacketArray = realloc(filtered->PacketArray, NumberOfPackets * sizeof(struct packet_t));
            }
        }
    }
    filtered->packet_count = FilteredCount;
    return 0;
}


uint64_t get_full_add(const uint8_t ip_address[4]) {
    uint64_t address = 0;
    for (int i = 0; i < 4; ++i) {
        address += ip_address[i];
        address *= 1000;
    }
    address /= 1000;
    return address;
}


uint64_t get_bit_mask(uint8_t mask_length) {
    uint32_t mask = ~((1 << (32 - mask_length)) - 1);
    return mask;
}

uint64_t get_bit_add(const uint8_t ip_address[4]) {
    uint32_t address = (ip_address[0] << 24) + (ip_address[1] << 16) + (ip_address[2] << 8) + ip_address[3];
    return address;
}

uint64_t get_startig_add(uint8_t ip_address[4], uint8_t mask_length) {
    uint32_t Bitmask = get_bit_mask(mask_length);
    uint32_t BitAnd = get_bit_add(ip_address) & Bitmask;
    return BitAnd;
}

uint64_t get_ending_add(uint8_t ip_address[4], uint8_t mask_length) {
    uint32_t Bitmask = get_bit_mask(mask_length);
    Bitmask = ~Bitmask;
    uint32_t BitAdress = get_startig_add(ip_address, mask_length);
    uint32_t BitAnd = BitAdress + Bitmask;
    return BitAnd;
}

int filter_from_to(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t source_ip[4],
        uint8_t destination_ip[4]) {

    filtered->pcapHeader = malloc(sizeof(struct pcap_header_t));
    if (error_check_mask(original,filtered) != 0){
        return EXIT_FAILURE;
    }
    if(memcpy(filtered->pcapHeader, original->pcapHeader, sizeof(struct packet_header_t)) == NULL){
        free(filtered->pcapHeader);
        return EXIT_FAILURE;

    }
    size_t NumberOfPackets = 10;
    size_t count = packet_count(original);
    size_t FilteredCount = 0;
    filtered->PacketArray = malloc(NumberOfPackets * sizeof(struct packet_t));
    if (filtered->PacketArray == NULL) {
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < count; ++i) {

        if ((get_full_add(original->PacketArray[i]->ip_header->src_addr) == get_full_add(source_ip))
            && (get_full_add(original->PacketArray[i]->ip_header->dst_addr) == get_full_add(destination_ip))) {
            filtered->PacketArray[FilteredCount] = malloc(sizeof(struct packet_t));
            if (filtered->PacketArray[FilteredCount] == NULL) {
                return EXIT_FAILURE;
            }
            if (copy_packet(original->PacketArray[i], filtered->PacketArray[FilteredCount])) {
                free(filtered -> pcapHeader);
                free(filtered->PacketArray);
                return EXIT_FAILURE;
            }
            FilteredCount++;
            if ((NumberOfPackets - 1) == FilteredCount) {
                NumberOfPackets *= 2;
                filtered->PacketArray = realloc(filtered->PacketArray, NumberOfPackets * sizeof(struct packet_t));
            }
        }
    }
    filtered->packet_count = FilteredCount;
    return 0;
}

int filter_from_mask(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t network_prefix[4],
        uint8_t mask_length) {

    if (error_check_mask(original,filtered) != 0){
        return EXIT_FAILURE;
    }

    filtered->pcapHeader = malloc(sizeof(struct pcap_header_t));
    if (filtered->pcapHeader == NULL) {
        return EXIT_FAILURE;
    }
    if(memcpy(filtered->pcapHeader, original->pcapHeader, sizeof(struct packet_header_t)) == NULL){
        free(filtered->pcapHeader);
        return EXIT_FAILURE;

    }
    bool allpackets = false;
    size_t NumberOfPackets = 10;
    filtered->packet_count = 0;
    size_t count = packet_count(original);
    size_t FilteredCount = 0;
    filtered->PacketArray = malloc(NumberOfPackets * sizeof(struct packet_t));
    if (filtered->PacketArray == NULL) {
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < count; ++i) {
        if (mask_length == 0) {
            allpackets = true;
        }
        if (mask_length == 32){
            continue;
        }
        uint64_t srcadd = get_bit_add(original->PacketArray[i]->ip_header->src_addr);
        uint64_t StartingAdress = get_startig_add(network_prefix, mask_length);
        uint64_t EndingAdress = get_ending_add(network_prefix, mask_length);
        if (((srcadd > StartingAdress) &&
             (srcadd <= EndingAdress)) || allpackets) {
            filtered->PacketArray[FilteredCount] = malloc(sizeof(struct packet_t));
            if (filtered->PacketArray[FilteredCount] == NULL) {
                return EXIT_FAILURE;
            }
            if (copy_packet(original->PacketArray[i], filtered->PacketArray[FilteredCount])) {
                free(filtered -> pcapHeader);
                free(filtered->PacketArray);
                return EXIT_FAILURE;
            }
            FilteredCount++;
            if ((NumberOfPackets - 1) == FilteredCount) {
                NumberOfPackets *= 2;
                filtered->PacketArray = realloc(filtered->PacketArray, NumberOfPackets * sizeof(struct packet_t));
            }

        } else {
            continue;
        }
    }
    filtered->packet_count = FilteredCount;
    return 0;
}

int filter_to_mask(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t network_prefix[4],
        uint8_t mask_length) {

    if (error_check_mask(original,filtered) != 0){
        return EXIT_FAILURE;
    }

    filtered->pcapHeader = malloc(sizeof(struct pcap_header_t));
    if (filtered->pcapHeader == NULL) {
        return EXIT_FAILURE;
    }
    if(memcpy(filtered->pcapHeader, original->pcapHeader, sizeof(struct packet_header_t)) == NULL){
            free(filtered->pcapHeader);
            return EXIT_FAILURE;

    }
    bool allpackets = false;
    size_t NumberOfPackets = 10;
    filtered->packet_count = 0;
    size_t count = packet_count(original);
    size_t FilteredCount = 0;
    filtered->PacketArray = malloc(NumberOfPackets * sizeof(struct packet_t));
    if (filtered->PacketArray == NULL) {
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < count; ++i) {


        if (mask_length == 0) {
            allpackets = true;
        }
        if (mask_length == 32){
            continue;
        }

        uint64_t srcadd = get_bit_add(original->PacketArray[i]->ip_header->dst_addr);
        uint64_t StartingAdress = get_startig_add(network_prefix, mask_length);
        uint64_t EndingAdress = get_ending_add(network_prefix, mask_length);

        if (((srcadd > StartingAdress) &&
             (srcadd <= EndingAdress)) || allpackets) {
            filtered->PacketArray[FilteredCount] = malloc(sizeof(struct packet_t));
            if (filtered->PacketArray[FilteredCount] == NULL) {
                return EXIT_FAILURE;
            }
            if (copy_packet(original->PacketArray[i], filtered->PacketArray[FilteredCount])) {
                free(filtered -> pcapHeader);
                free(filtered->PacketArray);
                return EXIT_FAILURE;
            }
            FilteredCount++;
            if ((NumberOfPackets - 1) == FilteredCount) {
                NumberOfPackets *= 2;
                filtered->PacketArray = realloc(filtered->PacketArray, NumberOfPackets * sizeof(struct packet_t));
            }

        } else {
            continue;
        }
    }
    filtered->packet_count = FilteredCount;
    return 0;
}

int count_ip_add(uint64_t IndexIpAddSrc, uint64_t IndexIpAddDst, const struct capture_t *const capture) {
    int count = 0;
    for (size_t i = 0; i < capture->packet_count; ++i) {
        if ((
                    get_full_add(capture->PacketArray[IndexIpAddSrc]->ip_header->src_addr) ==
                    get_full_add(capture->PacketArray[i]->ip_header->src_addr)) &&
            (get_full_add(capture->PacketArray[IndexIpAddDst]->ip_header->dst_addr) ==
             get_full_add(capture->PacketArray[i]->ip_header->dst_addr))) {
            count++;
        }
    }
    return count;
}

int print_ip_add(uint8_t add[4]) {
    printf("%" PRIu8 ".", add[0]);
    printf("%" PRIu8 ".", add[1]);
    printf("%" PRIu8 ".", add[2]);
    printf("%" PRIu8, add[3]);
    return 0;
}

int print_flow_stats(const struct capture_t *const capture) {
    size_t count = packet_count(capture);
    int position = 0;
    size_t countFound = 0;
    uint64_t ValueArrayFlowSrc[count];
    uint64_t ValueArrayFlowDst[count];
    uint64_t IndexArrayFlow[count];
    uint64_t ArrayCount[count];
    bool IsInArray = false;
    // GOING TROUGH ADRRESSES TO FIND ONE THAT IS ABSENT IN OUR ARRAY
    for (size_t i = 0; i < count; ++i) {
        ValueArrayFlowSrc[i] = NONADRESS;
        ValueArrayFlowDst[i] = NONADRESS;
        IndexArrayFlow[i] = NONADRESS;
        ArrayCount[i] = NONADRESS;
    }
    for (size_t i = 0; i < count; ++i) {
        IsInArray = false;
        for (size_t j = 0; j < count; ++j) {
            if ((
                        ValueArrayFlowSrc[j] == get_full_add(capture->PacketArray[i]->ip_header->src_addr)) &&
                ValueArrayFlowDst[j] == get_full_add(capture->PacketArray[i]->ip_header->dst_addr)) {
                IsInArray = true;
            }
        }
        // FIRST ENCOUNTER IP ADDRESS
        if (!IsInArray) {
            IndexArrayFlow[position] = i;
            ValueArrayFlowSrc[position] = get_full_add(capture->PacketArray[i]->ip_header->src_addr);
            ValueArrayFlowDst[position] = get_full_add(capture->PacketArray[i]->ip_header->dst_addr);
            position++;
            countFound++;
        }
    }
    // PRINTING
    for (size_t i = 0; i < countFound; ++i) {
        // COUNT IP ADD
        ArrayCount[i] = count_ip_add(IndexArrayFlow[i], IndexArrayFlow[i], capture);
        if (countFound == 0){
            return 0;
        }
        if (IndexArrayFlow[i] != NONADRESS) {
            print_ip_add(capture->PacketArray[IndexArrayFlow[i]]->ip_header->src_addr);
            printf(" -> ");
            print_ip_add(capture->PacketArray[IndexArrayFlow[i]]->ip_header->dst_addr);
            printf(" : %" PRIu64, ArrayCount[i]);
            printf("\n");
        }
    }

    return 0;
}

uint32_t find_max_interval(const uint32_t IntervalArray[], const uint32_t UIntervalArray[], size_t count) {
    uint32_t MaxInterval = 0;
    uint32_t Max = 0;
    uint32_t MaxU = 0;
    for (size_t i = 0; i < count; ++i) {
        if (Max <= IntervalArray[i]) {
            if (MaxU < UIntervalArray[i] || Max < IntervalArray[i]) {
                MaxU = UIntervalArray[i];
                MaxInterval = i;
            }
            Max = IntervalArray[i];
        }
    }
    return MaxInterval;
}

int print_longest_flow(const struct capture_t *const capture) {
    size_t count = packet_count(capture);
    int position = 0;
    size_t countFound = 0;
    uint64_t ValueArrayFlowSrc[count];
    uint64_t ValueArrayFlowDst[count];

    bool IsInArray = false;
    // GOING TROUGH ADRRESSES TO FIND ONE THAT IS ABSENT IN OUR ARRAY
    for (size_t i = 0; i < count; ++i) {
        ValueArrayFlowSrc[i] = NONADRESS;
        ValueArrayFlowDst[i] = NONADRESS;
    }
    for (size_t i = 0; i < count; ++i) {
        IsInArray = false;
        for (size_t j = 0; j < count; ++j) {

            if ((
                        ValueArrayFlowSrc[j] == get_full_add(capture->PacketArray[i]->ip_header->src_addr)) &&
                ValueArrayFlowDst[j] == get_full_add(capture->PacketArray[i]->ip_header->dst_addr)) {
                IsInArray = true;
            }

        }
        // FIRST ENCOUNTER IP ADDRESS
        if (!IsInArray) {

            ValueArrayFlowSrc[position] = get_full_add(capture->PacketArray[i]->ip_header->src_addr);
            ValueArrayFlowDst[position] = get_full_add(capture->PacketArray[i]->ip_header->dst_addr);
            position++;
            countFound++;
        }
    }
    // TIME SORTING
    uint32_t IntervalArray[countFound];
    uint32_t UIntervalArray[countFound];
    uint32_t IntervalArrayValuesStart[countFound];
    uint32_t IntervalArrayValuesEnd[countFound];
    uint32_t UIntervalArrayValuesStart[countFound];
    uint32_t UIntervalArrayValuesEnd[countFound];
    for (size_t i = 0; i < countFound; ++i) {
        uint32_t UIntervalStart = UINT32_MAX;
        uint32_t IntervalStart = UINT32_MAX;
        uint32_t IntervalEnd = 0;
        uint32_t UIntervalEnd = 0;
        for (size_t j = 0; j < count; ++j) {
            uint32_t ts_sec = capture->PacketArray[j]->packet_header->ts_sec;
            uint32_t ts_usec = capture->PacketArray[j]->packet_header->ts_usec;
            if (ValueArrayFlowSrc[i] == get_full_add(capture->PacketArray[j]->ip_header->src_addr) &&
                ValueArrayFlowDst[i] == get_full_add(capture->PacketArray[j]->ip_header->dst_addr)) {
                if ((IntervalStart > ts_sec) || ((IntervalStart == ts_sec) && (IntervalStart < ts_usec))) {
                    IntervalStart = ts_sec;
                    if (UIntervalStart > ts_usec) {
                        UIntervalStart = ts_usec;
                    }
                }
                if ((IntervalEnd < ts_sec) || ((IntervalEnd == ts_sec) && (UIntervalEnd < ts_usec))) {
                    IntervalEnd = ts_sec;
                    UIntervalEnd = ts_usec;


                }


                IntervalArray[i] = IntervalEnd - IntervalStart;
                UIntervalArray[i] = UIntervalEnd - UIntervalStart;
                UIntervalArrayValuesStart[i] = UIntervalStart;
                UIntervalArrayValuesEnd[i] = UIntervalEnd;
                IntervalArrayValuesStart[i] = IntervalStart;
                IntervalArrayValuesEnd[i] = IntervalEnd;
            }
        }
    }
    // MAX INTERVAL
    uint32_t MaxInterval = find_max_interval(IntervalArray, UIntervalArray, countFound);

    // PRINTING
    for (size_t i = 0; i < count; ++i) {
        if ((ValueArrayFlowSrc[MaxInterval] == get_full_add(capture->PacketArray[i]->ip_header->src_addr))
            && ValueArrayFlowDst[MaxInterval] == get_full_add(capture->PacketArray[i]->ip_header->dst_addr)
        ) {
            print_ip_add(capture->PacketArray[i]->ip_header->src_addr);
            printf(" -> ");
            print_ip_add(capture->PacketArray[i]->ip_header->dst_addr);
            printf(" : ");
            printf("%" PRIu32 ":%" PRIu32, IntervalArrayValuesStart[MaxInterval],
                   UIntervalArrayValuesStart[MaxInterval]);
            printf(" - ");
            printf("%" PRIu32 ":%" PRIu32, IntervalArrayValuesEnd[MaxInterval], UIntervalArrayValuesEnd[MaxInterval]);
            printf("\n");
            return 0;
        }
    }
    fprintf(stderr, "Empty capture received.\n");
    return EXIT_FAILURE;

}
