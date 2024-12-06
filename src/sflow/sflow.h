//
// Created by Francesco Ferreri (Namex) on 05/12/24.
//

#ifndef SFLOW_H
#define SFLOW_H
#define MAX_SFLOW_DATA 16384

#define SFLOW_FLOW_SAMPLE_FORMAT 0x00000001
#define SFLOW_RAW_PACKET_HEADER_FORMAT 0x00000001


/* RAW packet */

struct raw_packet_header {
  unsigned int header_protocol;
  unsigned int frame_length;
  unsigned int stripped;
  unsigned int header_length;
};

typedef struct raw_packet {
  struct raw_packet_header header;
  void *data;
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

#endif //SFLOW_H
