//
// Created by Francesco Ferreri (Namex) on 05/12/24.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "sflow.h"
#include "net.h"

sflow_datagram_t *sflow_decode_datagram(sflow_raw_data_t *sflow_raw_data) {

  	char *datagram = sflow_raw_data->data;

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

                if(ntohs(flow_record->header.data_format) == SFLOW_RAW_PACKET_HEADER_FORMAT) {
                	/* raw packet header follows */
                  	raw_packet_t *raw_packet = (raw_packet_t*)malloc(sizeof(raw_packet_t));
                 	memcpy(&raw_packet->header, datagram, sizeof(struct raw_packet_header));
                  	datagram += sizeof(struct raw_packet_header);

                    /* datalink section */
                  	datalink_header_t *datalink_header = (datalink_header_t*)malloc(sizeof(datalink_header_t));
                  	memcpy(&datalink_header->ethernet, datagram, sizeof(struct ethernet_header));
                  	datagram += sizeof(struct ethernet_header);

                  	if (ntohs(datalink_header->ethernet.ethertype) == ETHERTYPE_8021Q) {
                        datagram -= sizeof(unsigned short);
                        memcpy(&datalink_header->vlan, datagram, sizeof(struct vlan_header));
                        datagram += sizeof(struct vlan_header);
                        memcpy(&datalink_header->ethernet.ethertype, datagram, sizeof(unsigned short));
                        datagram += sizeof(unsigned short);
                  	}

                    raw_packet->datalink = datalink_header;

                    /* network section */
                    if (ntohs(datalink_header->ethernet.ethertype) == ETHERTYPE_IPV4) {
						ipv4_header_t *ipv4_header = (ipv4_header_t*)malloc(sizeof(ipv4_header_t));
                        memcpy(ipv4_header, datagram, sizeof(ipv4_header_t));
                        datagram += sizeof(ipv4_header_t);
						raw_packet->ipv4 = ipv4_header;
						raw_packet->ipv6 = NULL;
                    }
                    else if (ntohs(datalink_header->ethernet.ethertype) == ETHERTYPE_IPV6) {
                    	ipv6_header_t *ipv6_header = (ipv6_header_t*)malloc(sizeof(ipv6_header_t));
                        memcpy(ipv6_header, datagram, sizeof(ipv6_header_t));
                        datagram += sizeof(ipv6_header_t);
						raw_packet->ipv4 = NULL;
						raw_packet->ipv6 = ipv6_header;
                    }
                    else {
                      	/* do something about non-IP packet */
                        raw_packet->ipv6 = NULL;
                      	raw_packet->ipv4 = NULL;
                    }

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
    return sflow_datagram;
}

int sflow_free_datagram(sflow_datagram_t *sflow_datagram) {
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

storable_flow_t	*sflow_encode_flow_record(flow_record_t *record) {

  	storable_flow_t	*flow = (storable_flow_t*)malloc(sizeof(storable_flow_t));
	raw_packet_t 	*pkt = record->packet;

	flow->timestamp = time(NULL);

	char *dst_mac = pkt->datalink->ethernet.destination_mac;
    char *src_mac = pkt->datalink->ethernet.source_mac;

    snprintf(flow->dst_mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
         dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);

    snprintf(flow->src_mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
         src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);

    flow->proto = ntohs(pkt->datalink->ethernet.ethertype);

    if (flow->proto == ETHERTYPE_IPV4) {
      	char *addr_str;
     	struct in_addr source_address;
        struct in_addr destination_address;

        memcpy(&source_address.s_addr, pkt->ipv4->source_address, sizeof(unsigned int));
        memcpy(&destination_address.s_addr, pkt->ipv4->destination_address, sizeof(unsigned int));

        addr_str = inet_ntoa(source_address);
        memcpy(flow->src_ip, addr_str, sizeof(addr_str));

        addr_str = inet_ntoa(destination_address);
        memcpy(flow->dst_ip, addr_str, sizeof(addr_str));

        flow->size = pkt->ipv4->length + 14 + 20;
    }
    else {

    }


    return flow;
}