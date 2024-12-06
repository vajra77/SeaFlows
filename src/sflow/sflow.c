//
// Created by Francesco Ferreri (Namex) on 05/12/24.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sflow.h"

int sflow_decode_datagram(char *datagram, int len) {

    sflow_datagram_t *sflow_datagram = (sflow_datagram_t*)malloc(sizeof(sflow_datagram_t));
    memcpy(&sflow_datagram->header, datagram, sizeof(struct sflow_datagram_header));
    datagram += sizeof(struct sflow_datagram_header);

    /* samples loop */
    for (int n = 0; n < sflow_datagram->header.num_samples; n++) {
        unsigned int sample_format;
        memcpy(&sample_format, datagram, sizeof(unsigned int));

        if(sample_format & SFLOW_FLOW_SAMPLE_FORMAT) {
            /* a flow sample follows */
            flow_sample_t *flow_sample = (flow_sample_t*)malloc(sizeof(flow_sample_t));
            memcpy(&flow_sample->header, datagram, sizeof(struct flow_sample_header));
            flow_sample->next = NULL;
            datagram += sizeof(struct flow_sample_header);

            /* records loop */
            for (int k = 0; k < flow_sample->header.num_records; k++) {
                flow_record_t *flow_record = (flow_record_t*)malloc(sizeof(flow_record_t));
                memcpy(&flow_record->header, datagram, sizeof(struct flow_record_header));
                datagram += sizeof(struct flow_record_header);

                if(flow_record->header.data_format & SFLOW_RAW_PACKET_HEADER_FORMAT) {
                  /* raw packet header follows */
                  raw_packet_t *raw_packet = (raw_packet_t*)malloc(sizeof(raw_packet_t));
                  memcpy(&raw_packet->header, datagram, sizeof(struct raw_packet_header));
                  datagram += sizeof(struct raw_packet_header);

                  /*
                  *     parse ethernet/ip/tcp/udp
                  *                                */

                  flow_record->packet = raw_packet;

                  flow_record_t *ptr = flow_sample->records;
                  while (ptr != NULL) {
                      ptr = ptr->next;
                  }
                  ptr = flow_record;
                }
            }
            /* end of records loop */

            flow_sample_t *ptr = sflow_datagram->samples;
            while (ptr != NULL) {
                ptr = ptr->next;
            }
            ptr = flow_sample;
        }

    }
    /* end of samples loop */
    return 0;
}

int free_sflow_datagram(sflow_datagram_t *sflow_datagram) {
    flow_sample_t *pts = sflow_datagram->samples;
    while (pts != NULL) {
      flow_record_t *ptr = pts->records;
      while (ptr != NULL) {
        flow_record_t *fptr = ptr;
        ptr = ptr->next;
        free(fptr);
      }
      flow_sample_t *fpts = pts;
      pts = pts->next;
      free(fpts);
    }
    return 0;
}