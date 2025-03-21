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
#include <stdarg.h>

#include "sflow.h"

#include <sys/syslog.h>

#include "net.h"

int memguard(const void *ptr, const void *start, const ssize_t len, const int argc, ...) {

#ifdef SFLOW_DEBUG
	if (ptr > (start + len)) {
		syslog(LOG_ERR, "memguard: ptr overflow");
		va_list ap;
		va_start(ap, argc);
		for (int i = 1; i < argc; i++) {
			free(va_arg(ap, void *));
		}
		va_end(ap);
		return 1;
	}
	return 0;
#else
	return 0
#endif
}

/* full sFlow datagram decoding routine */
sflow_datagram_t *sflow_decode_datagram(const char *raw_data, const ssize_t raw_data_len) {

	const char *data_ptr = raw_data;
	uint32_t	buffer = 0x0;

    sflow_datagram_t *datagram = malloc(sizeof(sflow_datagram_t));
	bzero(datagram, sizeof(sflow_datagram_t));

	/* sFlow version */
    memcpy(&buffer, data_ptr, sizeof(uint32_t));
	datagram->header.version = ntohl(buffer);
    data_ptr += sizeof(uint32_t);
	if (memguard(data_ptr, raw_data, raw_data_len, 1, datagram)) return NULL;

	/* IP version */
    memcpy(&buffer, data_ptr, sizeof(uint32_t));
	datagram->header.ip_version = ntohl(buffer);
    data_ptr += sizeof(uint32_t);
	if (memguard(data_ptr, raw_data, raw_data_len, 1, datagram)) return NULL;

	/* agent address */
	if (datagram->header.ip_version == 1) {
		inet_ntop(AF_INET, data_ptr, datagram->header.agent_address, 255);
		data_ptr += sizeof(uint32_t);
		if (memguard(data_ptr, raw_data, raw_data_len, 1, datagram)) return NULL;
	} else {
		inet_ntop(AF_INET6, data_ptr, datagram->header.agent_address, 255);
		data_ptr += sizeof(uint32_t) * 4;
		if (memguard(data_ptr, raw_data, raw_data_len, 1, datagram)) return NULL;
	}

	/* sub agent id */
	memcpy(&buffer, data_ptr, sizeof(uint32_t));
	datagram->header.sub_agent_id = ntohl(buffer);
	data_ptr += sizeof(uint32_t);
	if (memguard(data_ptr, raw_data, raw_data_len, 1, datagram)) return NULL;

	/* datagram sequence number */
	memcpy(&buffer, data_ptr, sizeof(uint32_t));
	datagram->header.sequence_number = ntohl(buffer);
	data_ptr += sizeof(uint32_t);
	if (memguard(data_ptr, raw_data, raw_data_len, 1, datagram)) return NULL;

	/* switch uptime */
	memcpy(&buffer, data_ptr, sizeof(uint32_t));
	datagram->header.switch_uptime = ntohl(buffer);
	data_ptr += sizeof(uint32_t);
	if (memguard(data_ptr, raw_data, raw_data_len, 1, datagram)) return NULL;

	/* n of samples */
	memcpy(&buffer, data_ptr, sizeof(uint32_t));
	datagram->header.num_samples = ntohl(buffer);
	data_ptr += sizeof(uint32_t);
	if (memguard(data_ptr, raw_data, raw_data_len, 1, datagram)) return NULL;

    /* samples loop */
    for (int n = 0; n < datagram->header.num_samples; n++) {
    	syslog(LOG_DEBUG, "sflow: sample %d of %d", n, datagram->header.num_samples);

    	const char *sample_data_start = data_ptr;

    	/* sample format */
        memcpy(&buffer, data_ptr, sizeof(uint32_t));
		data_ptr += sizeof(uint32_t);
    	if (memguard(data_ptr, raw_data, raw_data_len, 1, datagram)) return NULL;

        if(ntohl(buffer) & SFLOW_FLOW_SAMPLE_FORMAT) {
            flow_sample_t *sample = malloc(sizeof(flow_sample_t));
        	bzero(sample, sizeof(flow_sample_t));

        	sample->header.data_format = ntohl(buffer);

        	/* sample length */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
        	sample->header.length = ntohl(buffer);
        	data_ptr += sizeof(uint32_t);
			if (memguard(data_ptr, raw_data, raw_data_len, 2, datagram, sample)) return NULL;

        	/* sample sequence number */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
        	sample->header.sequence_number = ntohl(buffer);
        	data_ptr += sizeof(uint32_t);
			if (memguard(data_ptr, raw_data, raw_data_len, 2, datagram, sample)) return NULL;

        	/* sample source id type/value */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
        	sample->header.source_id = ntohl(buffer);
        	data_ptr += sizeof(uint32_t);
			if (memguard(data_ptr, raw_data, raw_data_len, 2, datagram, sample)) return NULL;

        	/* sampling rate */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
        	sample->header.sampling_rate = ntohl(buffer);
        	data_ptr += sizeof(uint32_t);
			if (memguard(data_ptr, raw_data, raw_data_len, 2, datagram, sample)) return NULL;

        	/* sample pool */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
        	sample->header.sample_pool = ntohl(buffer);
        	data_ptr += sizeof(uint32_t);
			if (memguard(data_ptr, raw_data, raw_data_len, 2, datagram, sample)) return NULL;

        	/* drops */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
        	sample->header.drops = ntohl(buffer);
        	data_ptr += sizeof(uint32_t);
			if (memguard(data_ptr, raw_data, raw_data_len, 2, datagram, sample)) return NULL;

        	/* input interface */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
        	sample->header.input_interface = ntohl(buffer);
        	data_ptr += sizeof(uint32_t);
			if (memguard(data_ptr, raw_data, raw_data_len, 2, datagram, sample)) return NULL;

        	/* output interface */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
        	sample->header.output_interface = ntohl(buffer);
        	data_ptr += sizeof(uint32_t);
			if (memguard(data_ptr, raw_data, raw_data_len, 2, datagram, sample)) return NULL;

        	/* n records */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
        	sample->header.num_records = ntohl(buffer);
        	data_ptr += sizeof(uint32_t);
			if (memguard(data_ptr, raw_data, raw_data_len, 2, datagram, sample)) return NULL;
			syslog(LOG_DEBUG, "number of records in sample: %d", sample->header.num_records);

            /* records loop */
            for (int k = 0; k < sample->header.num_records; k++) {

            	const char *record_data_start = data_ptr;

            	flow_record_t *record = malloc(sizeof(flow_record_t));
            	bzero(record, sizeof(flow_record_t));

            	/* data format */
            	memcpy(&buffer, data_ptr, sizeof(uint32_t));
            	record->header.data_format = ntohl(buffer);
            	data_ptr += sizeof(uint32_t);
            	if (memguard(data_ptr, raw_data, raw_data_len, 3, datagram, sample, record)) return NULL;

            	/* flow data length */
            	memcpy(&buffer, data_ptr, sizeof(uint32_t));
            	record->header.length = ntohl(buffer);
            	data_ptr += sizeof(uint32_t);
            	if (memguard(data_ptr, raw_data, raw_data_len, 3, datagram, sample, record)) return NULL;


            	/* raw packet parser */
            	if (record->header.data_format & SFLOW_RAW_PACKET_HEADER_FORMAT) {
            		/* raw packet header */
            		raw_packet_t *packet = malloc(sizeof(raw_packet_t));

            		/* header protocol */
            		memcpy(&buffer, data_ptr, sizeof(uint32_t));
            		packet->header.protocol = ntohl(buffer);
            		data_ptr += sizeof(uint32_t);
            		if (memguard(data_ptr, raw_data, raw_data_len, 4, datagram, sample, record, packet)) return NULL;

            		/* frame length */
            		memcpy(&buffer, data_ptr, sizeof(uint32_t));
            		packet->header.frame_length = ntohl(buffer);
            		data_ptr += sizeof(uint32_t);
            		if (memguard(data_ptr, raw_data, raw_data_len, 4, datagram, sample, record, packet)) return NULL;

            		/* stripped */
            		memcpy(&buffer, data_ptr, sizeof(uint32_t));
            		packet->header.stripped = ntohl(buffer);
            		data_ptr += sizeof(uint32_t);
            		if (memguard(data_ptr, raw_data, raw_data_len, 4, datagram, sample, record, packet)) return NULL;

            		/* size */
            		memcpy(&buffer, data_ptr, sizeof(uint32_t));
            		packet->header.size = ntohl(buffer);
            		data_ptr += sizeof(uint32_t);
            		if (memguard(data_ptr, raw_data, raw_data_len, 4, datagram, sample, record, packet)) return NULL;

            		/* reset all packet data */
            		packet->datalink = NULL;
            		packet->ipv4 = NULL;
            		packet->ipv6 = NULL;

            		if (packet->header.protocol & SFLOW_RAW_PACKET_HEADER_PROTO_ETHERNET) {
            			/* ethernet header follows */
            			datalink_header_t *datalink = malloc(sizeof(datalink_header_t));

            			/* destination MAC address */
            			memcpy(datalink->ethernet.destination_mac, data_ptr, 6);
            			data_ptr += 6;
            			if (memguard(data_ptr, raw_data, raw_data_len, 5,
							datagram, sample, record, packet, datalink)) return NULL;

            			/* source MAC address */
            			memcpy(datalink->ethernet.source_mac, data_ptr, 6);
            			data_ptr += 6;
            			if (memguard(data_ptr, raw_data, raw_data_len, 5,
							datagram, sample, record, packet, datalink)) return NULL;

            			/* ethertype */
            			uint16_t	type_len;
            			memcpy(&type_len, data_ptr, sizeof(uint16_t));
            			data_ptr += sizeof(uint16_t);
            			if (memguard(data_ptr, raw_data, raw_data_len, 5,
							datagram, sample, record, packet, datalink)) return NULL;

            			if (ntohs(type_len) == ETHERTYPE_8021Q) {
            				/* vlan id */
            				uint16_t vlan;
            				memcpy(&vlan, data_ptr, sizeof(uint16_t));
            				datalink->vlan.id = ntohs(vlan);
            				datalink->vlan.length = 0;
            				data_ptr += sizeof(uint16_t);
            				if (memguard(data_ptr, raw_data, raw_data_len, 5,
								datagram, sample, record, packet, datalink)) return NULL;

            				/* re-read shifted type_len */
            				memcpy(&type_len, data_ptr, sizeof(uint16_t));
            				data_ptr += sizeof(uint16_t);
            				if (memguard(data_ptr, raw_data, raw_data_len, 5,
								datagram, sample, record, packet, datalink)) return NULL;
            			}

            			if (ntohs(type_len) == ETHERTYPE_IPV4) {
            				datalink->ethernet.ethertype = ETHERTYPE_IPV4;
            				datalink->vlan.id = 0;
            				datalink->vlan.length = 0;

            				ipv4_header_t *ipv4 = malloc(sizeof(ipv4_header_t));

            				/* total length */
            				memcpy(&buffer, data_ptr, sizeof(uint32_t));
            				ipv4->preamble = ntohl(buffer) & 0xffff0000;
            				ipv4->length = ntohl(buffer) & 0x0000ffff;
            				data_ptr += sizeof(uint32_t);
            				if (memguard(data_ptr, raw_data, raw_data_len, 6,
								datagram, sample, record, packet, datalink, ipv4)) return NULL;

            				/* ttl/protocol */
            				memcpy(&buffer, data_ptr, sizeof(uint32_t));
            				ipv4->ttl = (ntohl(buffer) & 0xff000000) >> 6;
            				ipv4->protocol = (ntohl(buffer) & 0x00ff0000) >> 4;
            				data_ptr += sizeof(uint32_t);
            				if (memguard(data_ptr, raw_data, raw_data_len, 6,
								datagram, sample, record, packet, datalink, ipv4)) return NULL;

            				/* src address */
            				inet_ntop(AF_INET, data_ptr, ipv4->source_address, 256);
            				data_ptr += 6;
            				if (memguard(data_ptr, raw_data, raw_data_len, 6,
								datagram, sample, record, packet, datalink, ipv4)) return NULL;

            				/* dst address */
            				inet_ntop(AF_INET, data_ptr, ipv4->source_address, 256);
            				data_ptr += 6;
            				if (memguard(data_ptr, raw_data, raw_data_len, 6,
								datagram, sample, record, packet, datalink, ipv4)) return NULL;

            				packet->datalink = datalink;
            				packet->ipv4 = ipv4;
            			} else if (ntohs(type_len) == ETHERTYPE_IPV6) {
            				datalink->ethernet.ethertype = ETHERTYPE_IPV6;
            				datalink->vlan.id = 0;
            				datalink->vlan.length = 0;

            				ipv6_header_t *ipv6 = malloc(sizeof(ipv6_header_t));

            				/* preamble */
            				memcpy(&buffer, data_ptr, sizeof(uint32_t));
            				ipv6->preamble = ntohl(buffer);
            				data_ptr += sizeof(uint32_t);
            				if (memguard(data_ptr, raw_data, raw_data_len, 6,
								datagram, sample, record, packet, datalink, ipv6)) return NULL;

            				/* length */
            				memcpy(&buffer, data_ptr, sizeof(uint32_t));
            				ipv6->length = (ntohl(buffer) & 0xffff0000) >> 4;
            				data_ptr += sizeof(uint32_t);
            				if (memguard(data_ptr, raw_data, raw_data_len, 6,
								datagram, sample, record, packet, datalink, ipv6)) return NULL;

            				/* src address */
            				inet_ntop(AF_INET6, data_ptr, ipv6->source_address, 256);
            				data_ptr += 16;
            				if (memguard(data_ptr, raw_data, raw_data_len, 6,
								datagram, sample, record, packet, datalink, ipv6)) return NULL;

            				/* dst address */
            				inet_ntop(AF_INET6, data_ptr, ipv6->source_address, 256);
            				data_ptr += 16;
            				if (memguard(data_ptr, raw_data, raw_data_len, 6,
								datagram, sample, record, packet, datalink, ipv6)) return NULL;

            				packet->datalink = datalink;
            				packet->ipv6 = ipv6;

            			} else {
            				packet->datalink = datalink;
            			}
            		}
            		record->packet = packet;
            	} /* end of raw packet parser */

            	/* add record to sample */
        		syslog(LOG_DEBUG, "Adding record to datagram");
            	if (sample->records) {
            		flow_record_t *last = sample->records;
            		flow_record_t *current = sample->records;
            		while (current) {
            			last = current;
            			current = current->next;
            		}
            		record->next = NULL;
            		last->next = record;
				} else {
            		record->next = NULL;
					sample->records = record;
				}

            	/* align pointer for next record */
            	if (data_ptr < record_data_start + record->header.length) {
            		data_ptr = record_data_start + record->header.length;
            	}
            } /* end of records loop */

        	/* add sample to datagram */
        	syslog(LOG_DEBUG, "Adding sample to datagram");
        	if (datagram->samples) {
        		flow_sample_t *last = datagram->samples;
        		flow_sample_t *current = datagram->samples;
        		while (current) {
        			last = current;
        			current = current->next;
        		}
        		sample->next = NULL;
        		last->next = sample;
        	} else {
        		sample->next = NULL;
        		datagram->samples = sample;
        	}

        	/* align pointer for next sample */
        	if (data_ptr < sample_data_start + sample->header.length) {
        		data_ptr = sample_data_start + sample->header.length;
        	}
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