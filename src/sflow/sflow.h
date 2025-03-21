//
// Created by Francesco Ferreri (Namex) on 05/12/24.
//

#ifndef SFLOW_H
#define SFLOW_H

#include <sys/types.h>
#include "net.h"

#define MAX_SFLOW_DATA 16384

#define SFLOW_FLOW_SAMPLE_FORMAT                  0x00000001
#define SFLOW_RAW_PACKET_HEADER_FORMAT            0x00000001
#define SFLOW_RAW_PACKET_HEADER_PROTO_ETHERNET    0x00000001

#define SFLOW_DEBUG

#ifdef SFLOW_DEBUG
#define MEMGUARD(ptr, start, len) if (ptr > start + len) { syslog(LOG_WARNING, "memory overflow"); return NULL; }
#else
#define MEMGUARD(ptr, start, len)
#endif

#define MAC_ADDR_SIZE 14
#define IP_ADDR_SIZE 256

/* Storable Flow */

typedef struct storable_flow {
  time_t timestamp;
  char src_mac[MAC_ADDR_SIZE];
  char dst_mac[MAC_ADDR_SIZE];
  uint32_t proto;
  char src_ip[IP_ADDR_SIZE];
  char dst_ip[IP_ADDR_SIZE];
  uint32_t size;
  uint32_t sampling_rate;
  uint32_t computed_size;
} storable_flow_t;

/* RAW packet */

struct raw_packet_header {
  uint32_t protocol;
  uint32_t frame_length;
  uint32_t stripped;
  uint32_t size;
};

typedef struct raw_packet {
  struct raw_packet_header header;
  datalink_header_t		    *datalink;
  ipv4_header_t			      *ipv4;
  ipv6_header_t			      *ipv6;
} raw_packet_t;


/* Flow Record */

struct flow_record_header {
  uint32_t data_format;
  uint32_t length;
};

typedef struct flow_record {
  struct flow_record_header header;
  struct raw_packet *packet;
  struct flow_record *next;
} flow_record_t;


/* Flow Sample */

struct flow_sample_header {
  uint32_t data_format;
  uint32_t length;
  uint32_t sequence_number;
  uint32_t source_id;
  uint32_t sampling_rate;
  uint32_t sample_pool;
  uint32_t drops;
  uint32_t input_interface;
  uint32_t output_interface;
  uint32_t num_records;
};

typedef struct flow_sample {
  struct flow_sample_header header;
  struct flow_sample *next;
  flow_record_t *records;
} flow_sample_t;


/* sFlow Datagram */

struct sflow_datagram_header {
  uint32_t  version;
  uint32_t  ip_version;
  char      agent_address[255];
  uint32_t  sub_agent_id;
  uint32_t  sequence_number;
  uint32_t  switch_uptime;
  uint32_t  num_samples;
};

typedef struct sflow_datagram {
  struct sflow_datagram_header header;
  flow_sample_t *samples;
} sflow_datagram_t;

sflow_datagram_t* 	  sflow_decode_datagram(const char *, ssize_t);
storable_flow_t*	  sflow_encode_flow_record(const flow_record_t*, uint32_t);
void                  sflow_free_datagram(sflow_datagram_t*);

#endif //SFLOW_H
