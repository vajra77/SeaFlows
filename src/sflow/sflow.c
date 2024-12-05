//
// Created by Francesco Ferreri (Namex) on 05/12/24.
//
#include <stdio.h>
#include <string.h>

#include "sflow.h"

int sflow_decode_datagram(char *datagram, int len) {

    sflow_datagram_t *sflow_datagram = (sflow_datagram_t*)malloc(sizeof(sflow_datagram_t));
    memcpy(sflow_datagram->header, datagram, sizeof(struct sflow_datagram_header));
    datagram += sizeof(struct sflow_datagram_header);

    /* samples loop */
    for (int n = 0; n < sflow_datagram->num_samples; n++) {
        unsigned int sample_format;
        memcpy(&sample_format, datagram, sizeof(unsigned int));

        if(sample_format & SFLOW_FLOW_SAMPLE_FORMAT) {
            /* a flow sample follows */
            flow_sample_t *flow_sample = (flow_sample_t*)malloc(sizeof(flow_sample_t));
            memcpy(flow_sample->header, datagram, sizeof(struct flow_sample_header));
            flow_sample->next = NULL;
            datagram += sizeof(struct flow_sample_header);

            /* records loop */
            for (int k = 0; k < flow_sample->num_records; k++) {
                unsigned int record_format;
                memcpy(&record_format, datagram, sizeof(unsigned int));

                if(record_format & SFLOW_RAW_PACKET_HEADER_FORMAT) {
                  /* raw packet header follows */
                  raw_packet_t *raw_packet = (raw_packet_t*)malloc(sizeof(raw_packet_t));
                  memcpy(raw_packet->header, datagram, sizeof(struct raw_packet_header));
                  datagram += sizeof(struct raw_packet_header);


                }
            }
            /* end of records loop */

            if (sflow_datagram->samples == NULL) {
                  sflow_datagram->samples = flow_sample;
            }
            else {
                sflow_sample_t *ptr = sflow_datagram->samples
                while (ptr != NULL) {
                    ptr = ptr->next;
                }
                ptr = flow_sample;
            }
        }

    }
    /* end of samples loop */
}