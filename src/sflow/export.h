//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#ifndef PARSER_H
#define PARSER_H
#define MAX_SFLOW_DATA 16384

typedef struct storable_flow {
  unsigned int timestamp;
  char src_mac[6];
  char dst_mac[6];
  char src_ip[4];
  char dst_ip[4];
  unsigned int src_port;
  unsigned int dst_port;
  unsigned int protocol;
  unsigned int size;
} storable_flow_t;

typedef struct sflow_raw_data {
    char data[MAX_SFLOW_DATA];
    int size;
} sflow_raw_data_t;


#endif //PARSER_H
