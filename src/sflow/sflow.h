//
// Created by Francesco Ferreri (Namex) on 05/12/24.
//

#ifndef SFLOW_H
#define SFLOW_H
#include "net.h"

#define MAX_SFLOW_DATA 16384

#define SFLOW_FLOW_SAMPLE_FORMAT 0x00000001
#define SFLOW_RAW_PACKET_HEADER_FORMAT 0x00000001

/* Storable Flow */

typedef struct storable_flow {
  time_t timestamp;
  char src_mac[18];
  char dst_mac[18];
  unsigned int proto;
  char src_ip[18];
  char dst_ip[18];
  unsigned int size;
  unsigned int sampling_rate;
  unsigned int computed_size;
} storable_flow_t;

/* Sflow RAW Data */

typedef struct sflow_raw_data {
    char data[MAX_SFLOW_DATA];
    int size;
} sflow_raw_data_t;

/* RAW packet */

struct raw_packet_header {
  unsigned int header_protocol;
  unsigned int frame_length;
  unsigned int stripped;
  unsigned int header_length;
};

typedef struct raw_packet {
  struct raw_packet_header header;
  datalink_header_t		*datalink;
  ipv4_header_t			*ipv4;
  ipv6_header_t			*ipv6;
} raw_packet_t;


/* Flow Record */

struct flow_record_header {
  unsigned int data_format;
  unsigned int length;
};

typedef struct flow_record {
  struct flow_record_header header;
  struct raw_packet *packet;
  struct flow_record *next;
} flow_record_t;


/* Flow Sample */

struct flow_sample_header {
  unsigned int data_format;
  unsigned int length;
  unsigned int sequence_number;
  unsigned int source_id;
  unsigned int sampling_rate;
  unsigned int sample_pool;
  unsigned int drops;
  unsigned int input_interface;
  unsigned int output_interface;
  unsigned int num_records;
};

typedef struct flow_sample {
  struct flow_sample_header header;
  struct flow_sample *next;
  flow_record_t *records;
} flow_sample_t;


/* sFlow Datagram */

struct sflow_datagram_header {
  unsigned int version;
  unsigned int ip_version;
  unsigned int ipv4_address;
  unsigned int sub_agent_id;
  unsigned int sequence_number;
  unsigned int switch_uptime;
  unsigned int num_samples;
};

typedef struct sflow_datagram {
  struct sflow_datagram_header header;
  flow_sample_t *samples;
} sflow_datagram_t;

sflow_datagram_t* 	sflow_decode_datagram(const sflow_raw_data_t*);
storable_flow_t*	sflow_encode_flow_record(const flow_record_t*, unsigned int);
void sflow_free_datagram(sflow_datagram_t*);

#endif //SFLOW_H
