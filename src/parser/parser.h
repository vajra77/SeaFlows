//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#ifndef PARSER_H
#define PARSER_H
#define MAX_SFLOW_DATA 16384

typedef struct sflow_raw_data {
    char data[MAX_SFLOW_DATA];
    int size;
} sflow_raw_data_t;


#endif //PARSER_H
