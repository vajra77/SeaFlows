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


/* full sFlow datagram decoding routine */
sflow_datagram_t *sflow_decode_datagram(const sflow_raw_data_t *sflow_raw_data) {

  	const char *raw_data = sflow_raw_data->data;
	uint32_t	buffer = 0x0;

    sflow_datagram_t *datagram = malloc(sizeof(sflow_datagram_t));

	/* sFlow version */
    memcpy(&buffer, raw_data, sizeof(uint32_t));
	datagram->header.version = ntohl(buffer);
    raw_data += sizeof(uint32_t);

	/* IP version */
    memcpy(&buffer, raw_data, sizeof(uint32_t));
	datagram->header.ip_version = ntohl(buffer);
    raw_data += sizeof(uint32_t);

	/* agent address */
	if (datagram->header.ip_version == 1) {
		inet_ntop(AF_INET, raw_data, datagram->header.agent_address, 255);
		raw_data += sizeof(uint32_t);
	} else {
		inet_ntop(AF_INET6, raw_data, datagram->header.agent_address, 255);
		raw_data += sizeof(uint32_t) * 4;
	}

	/* sub agent id */
	memcpy(&buffer, raw_data, sizeof(uint32_t));
	datagram->header.sub_agent_id = ntohl(buffer);
	raw_data += sizeof(uint32_t);

	/* datagram sequence number */
	memcpy(&buffer, raw_data, sizeof(uint32_t));
	datagram->header.sequence_number = ntohl(buffer);
	raw_data += sizeof(uint32_t);

	/* switch uptime */
	memcpy(&buffer, raw_data, sizeof(uint32_t));
	datagram->header.switch_uptime = ntohl(buffer);
	raw_data += sizeof(uint32_t);

	/* n of samples */
	memcpy(&buffer, raw_data, sizeof(uint32_t));
	datagram->header.num_samples = ntohl(buffer);
	raw_data += sizeof(uint32_t);

    /* samples loop */
    for (int n = 0; n < datagram->header.num_samples; n++) {
    	/* sample format */
        memcpy(&buffer, raw_data, sizeof(uint32_t));
		raw_data += sizeof(uint32_t);

        if(ntohl(buffer) & SFLOW_FLOW_SAMPLE_FORMAT) {
            flow_sample_t *sample = malloc(sizeof(flow_sample_t));

        	/* sample length */
            memcpy(&buffer, raw_data, sizeof(uint32_t));
        	sample->header.data_format = ntohl(buffer);
        	raw_data += sizeof(uint32_t);

        	/* sample sequence number */
            memcpy(&buffer, raw_data, sizeof(uint32_t));
        	sample->header.sequence_number = ntohl(buffer);
        	raw_data += sizeof(uint32_t);

        	/* sample source id type/value */
            memcpy(&buffer, raw_data, sizeof(uint32_t));
        	sample->header.source_id = ntohl(buffer);
        	raw_data += sizeof(uint32_t);

        	/* sampling rate */
            memcpy(&buffer, raw_data, sizeof(uint32_t));
        	sample->header.sampling_rate = ntohl(buffer);
        	raw_data += sizeof(uint32_t);

        	/* sample pool */
            memcpy(&buffer, raw_data, sizeof(uint32_t));
        	sample->header.sample_pool = ntohl(buffer);
        	raw_data += sizeof(uint32_t);

        	/* drops */
            memcpy(&buffer, raw_data, sizeof(uint32_t));
        	sample->header.drops = ntohl(buffer);
        	raw_data += sizeof(uint32_t);

        	/* input interface */
            memcpy(&buffer, raw_data, sizeof(uint32_t));
        	sample->header.input_interface = ntohl(buffer);
        	raw_data += sizeof(uint32_t);

        	/* output interface */
            memcpy(&buffer, raw_data, sizeof(uint32_t));
        	sample->header.output_interface = ntohl(buffer);
        	raw_data += sizeof(uint32_t);

        	/* n records */
            memcpy(&buffer, raw_data, sizeof(uint32_t));
        	sample->header.num_records = ntohl(buffer);
        	raw_data += sizeof(uint32_t);

            sample->next = NULL;

            /* records loop */
            for (int k = 0; k < sample->header.num_records; k++) {
                flow_record_t *record = malloc(sizeof(flow_record_t));

            	record->packet = NULL;
            	record->next = NULL;

				/* data format */
            	memcpy(&buffer, raw_data, sizeof(uint32_t));
				record->header.data_format = ntohl(buffer);
            	raw_data += sizeof(uint32_t);

            	/* raw packet parser */
            	if (record->header.data_format & SFLOW_RAW_PACKET_HEADER_FORMAT) {
					/* raw packet header */
            		raw_packet_t *packet = malloc(sizeof(raw_packet_t));

            		/* header protocol */
            		memcpy(&buffer, raw_data, sizeof(uint32_t));
            		packet->header.protocol = ntohl(buffer);
            		raw_data += sizeof(uint32_t);

            		/* frame length */
            		memcpy(&buffer, raw_data, sizeof(uint32_t));
            		packet->header.frame_length = ntohl(buffer);
            		raw_data += sizeof(uint32_t);

            		/* stripped */
            		memcpy(&buffer, raw_data, sizeof(uint32_t));
            		packet->header.stripped = ntohl(buffer);
            		raw_data += sizeof(uint32_t);

            		/* size */
            		memcpy(&buffer, raw_data, sizeof(uint32_t));
            		packet->header.size = ntohl(buffer);
            		raw_data += sizeof(uint32_t);

            		/* reset all packet data */
            		packet->datalink = NULL;
            		packet->ipv4 = NULL;
            		packet->ipv6 = NULL;

            		if (packet->header.protocol & SFLOW_RAW_PACKET_HEADER_PROTO_ETHERNET) {
            			/* ethernet header follows */
            			datalink_header_t *datalink = malloc(sizeof(datalink_header_t));

            			/* destination MAC address */
            			memcpy(datalink->ethernet.destination_mac, raw_data, 6);
            			raw_data += 6;

            			/* source MAC address */
            			memcpy(datalink->ethernet.source_mac, raw_data, 6);
            			raw_data += 6;

            			/* ethertype */
            			uint16_t	type_len;
            			memcpy(&type_len, raw_data, sizeof(uint16_t));
            			raw_data += sizeof(uint16_t);

            			if (ntohs(type_len) == ETHERTYPE_8021Q) {
            				/* vlan id */
							uint16_t vlan;
            				memcpy(&vlan, raw_data, sizeof(uint16_t));
            				datalink->vlan.id = ntohs(vlan);
							datalink->vlan.length = 0;
            				raw_data += sizeof(uint16_t);

            				/* re-read shifted type_len */
            				memcpy(&type_len, raw_data, sizeof(uint16_t));
            				raw_data += sizeof(uint16_t);
            			}

            			if (ntohs(type_len) == ETHERTYPE_IPV4) {
							datalink->ethernet.ethertype = ETHERTYPE_IPV4;
							datalink->vlan.id = 0;
            				datalink->vlan.length = 0;

							ipv4_header_t *ipv4 = malloc(sizeof(ipv4_header_t));

            				/* total length */
            				memcpy(&buffer, raw_data, sizeof(uint32_t));
            				ipv4->preamble = ntohl(buffer) & 0xffff0000;
            				ipv4->length = ntohl(buffer) & 0x0000ffff;
            				raw_data += sizeof(uint32_t);

            				/* ttl/protocol */
            				memcpy(&buffer, raw_data, sizeof(uint32_t));
							ipv4->ttl = (ntohl(buffer) & 0xff000000) >> 6;
            				ipv4->protocol = (ntohl(buffer) & 0x00ff0000) >> 4;
							raw_data += sizeof(uint32_t);

            				/* src address */
            				inet_ntop(AF_INET, raw_data, ipv4->source_address, 256);
							raw_data += 6;

            				/* dst address */
            				inet_ntop(AF_INET, raw_data, ipv4->source_address, 256);
							raw_data += 6;

							packet->datalink = datalink;
							packet->ipv4 = ipv4;
						} else if (ntohs(type_len) == ETHERTYPE_IPV6) {
							datalink->ethernet.ethertype = ETHERTYPE_IPV6;
							datalink->vlan.id = 0;
							datalink->vlan.length = 0;

							ipv6_header_t *ipv6 = malloc(sizeof(ipv6_header_t));

							/* preamble */
							memcpy(&buffer, raw_data, sizeof(uint32_t));
							ipv6->preamble = ntohl(buffer);
							raw_data += sizeof(uint32_t);

							/* length */
							memcpy(&buffer, raw_data, sizeof(uint32_t));
							ipv6->length = (ntohl(buffer) & 0xffff0000) >> 4;
							raw_data += sizeof(uint32_t);

							/* src address */
							inet_ntop(AF_INET6, raw_data, ipv6->source_address, 256);
							raw_data += 16;

							/* dst address */
							inet_ntop(AF_INET6, raw_data, ipv6->source_address, 256);
							raw_data += 16;

							packet->datalink = datalink;
							packet->ipv6 = ipv6;

						} else {
							packet->datalink = datalink;
            			}
            		}
            		record->packet = packet;
            	} /* end of raw packet parser */

            	/* add record to sample */
            	flow_record_t *last = sample->records;
            	flow_record_t *current = sample->records;
            	while (current) {
            		last = current;
            		current = current->next;
            	}
            	last->next = record;
            } /* end of records loop */

        	/* add sample to datagram */
        	flow_sample_t *last = datagram->samples;
        	flow_sample_t *current = datagram->samples;
        	while (current) {
        		last = current;
        		current = current->next;
        	}
        	last->next = sample;
        }
    }  /* end of samples loop */

	return datagram;
}


void sflow_free_datagram(sflow_datagram_t *sflow_datagram) {
    flow_sample_t *pts = sflow_datagram->samples;
    while (pts != NULL) {
      flow_record_t *ptr = pts->records;
      while (ptr != NULL) {
        flow_record_t *fptr = ptr;
        ptr = ptr->next;
      	if (fptr->packet) {
      		if (fptr->packet->datalink) {
      			free(fptr->packet->datalink);
      		}
      		if (fptr->packet->ipv4) {
      			free(fptr->packet->ipv4);
      		}
      		if (fptr->packet->ipv6) {
      			free(fptr->packet->ipv6);
      		}
      		free(fptr->packet);
      	}
        free(fptr);
      }
      flow_sample_t *fpts = pts;
      pts = pts->next;
      free(fpts);
    }
	free(sflow_datagram);
}

storable_flow_t	*sflow_encode_flow_record(const flow_record_t *record, const uint32_t sampling_rate) {

  	storable_flow_t	*flow = malloc(sizeof(storable_flow_t));
	const raw_packet_t 	*pkt = record->packet;

	flow->timestamp = time(NULL);

	strcpy(flow->dst_mac, pkt->datalink->ethernet.destination_mac);
	strcpy(flow->src_mac, pkt->datalink->ethernet.source_mac);

    flow->proto = pkt->datalink->ethernet.ethertype;

    if (flow->proto == ETHERTYPE_IPV4) {
		strcpy(flow->src_ip, pkt->ipv4->source_address);
    	strcpy(flow->dst_ip, pkt->ipv4->source_address);
        flow->size = pkt->ipv4->length + 34;
    	flow->sampling_rate = sampling_rate;
    	flow->computed_size = flow->size * flow->sampling_rate;
    } else if (flow->proto == ETHERTYPE_IPV6) { /* IPv6 */
		strcpy(flow->src_ip, pkt->ipv6->source_address);
    	strcpy(flow->dst_ip, pkt->ipv6->source_address);
    	flow->size = pkt->ipv6->length + 40;
    	flow->sampling_rate = sampling_rate;
    	flow->computed_size = flow->size * flow->sampling_rate;
    }

    return flow;
}