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
#include <sys/syslog.h>

#include "gc/gc.h"
#include "sflow.h"
#include "net.h"


/* full sFlow datagram decoding routine */
sflow_datagram_t *sflow_decode_datagram(const char *raw_data, const ssize_t raw_data_len) {
    const char *data_ptr = raw_data;
    uint32_t buffer = 0x0;

    gc_t gc;
    gc_init(&gc);

    // sflow_datagram_t *datagram = malloc(sizeof(sflow_datagram_t));
    sflow_datagram_t *datagram = gc_alloc(&gc, sizeof(sflow_datagram_t));
    bzero(datagram, sizeof(sflow_datagram_t));

    /* sFlow version */
    memcpy(&buffer, data_ptr, sizeof(uint32_t));
    datagram->header.version = ntohl(buffer);
    data_ptr += sizeof(uint32_t);
    MEMGUARD(data_ptr, raw_data, raw_data_len);

    /* IP version */
    memcpy(&buffer, data_ptr, sizeof(uint32_t));
    datagram->header.ip_version = ntohl(buffer);
    data_ptr += sizeof(uint32_t);
    MEMGUARD(data_ptr, raw_data, raw_data_len);

    /* agent address */
    if (datagram->header.ip_version == 1) {
        inet_ntop(AF_INET, data_ptr, datagram->header.agent_address, 255);
        data_ptr += sizeof(uint32_t);
        MEMGUARD(data_ptr, raw_data, raw_data_len);
    } else {
        inet_ntop(AF_INET6, data_ptr, datagram->header.agent_address, 255);
        data_ptr += sizeof(uint32_t) * 4;
        MEMGUARD(data_ptr, raw_data, raw_data_len);
    }

    /* sub agent id */
    memcpy(&buffer, data_ptr, sizeof(uint32_t));
    datagram->header.sub_agent_id = ntohl(buffer);
    data_ptr += sizeof(uint32_t);
    MEMGUARD(data_ptr, raw_data, raw_data_len);

    /* datagram sequence number */
    memcpy(&buffer, data_ptr, sizeof(uint32_t));
    datagram->header.sequence_number = ntohl(buffer);
    data_ptr += sizeof(uint32_t);
    MEMGUARD(data_ptr, raw_data, raw_data_len);

    /* switch uptime */
    memcpy(&buffer, data_ptr, sizeof(uint32_t));
    datagram->header.switch_uptime = ntohl(buffer);
    data_ptr += sizeof(uint32_t);
    MEMGUARD(data_ptr, raw_data, raw_data_len);

    /* n of samples */
    memcpy(&buffer, data_ptr, sizeof(uint32_t));
    datagram->header.num_samples = ntohl(buffer);
    data_ptr += sizeof(uint32_t);
    MEMGUARD(data_ptr, raw_data, raw_data_len);

    /* samples loop */
    for (int n = 0; n < datagram->header.num_samples; n++) {
        // flow_sample_t *sample = malloc(sizeof(flow_sample_t));
        flow_sample_t *sample = gc_alloc(&gc, sizeof(flow_sample_t));
        bzero(sample, sizeof(flow_sample_t));

        /* sample format */
        memcpy(&buffer, data_ptr, sizeof(uint32_t));
        data_ptr += sizeof(uint32_t);
        sample->header.data_format = ntohl(buffer);
        MEMGUARD(data_ptr, raw_data, raw_data_len);

        /* sample length */
        memcpy(&buffer, data_ptr, sizeof(uint32_t));
        sample->header.length = ntohl(buffer);
        data_ptr += sizeof(uint32_t);
        MEMGUARD(data_ptr, raw_data, raw_data_len);

        const char *sample_data_start = data_ptr;

        if (sample->header.data_format & SFLOW_FLOW_SAMPLE_FORMAT) {
            /* sample sequence number */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
            sample->header.sequence_number = ntohl(buffer);
            data_ptr += sizeof(uint32_t);
            MEMGUARD(data_ptr, raw_data, raw_data_len);

            /* sample source id type/value */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
            sample->header.source_id = ntohl(buffer);
            data_ptr += sizeof(uint32_t);
            MEMGUARD(data_ptr, raw_data, raw_data_len);

            /* sampling rate */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
            sample->header.sampling_rate = ntohl(buffer);
            data_ptr += sizeof(uint32_t);
            MEMGUARD(data_ptr, raw_data, raw_data_len);

            /* sample pool */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
            sample->header.sample_pool = ntohl(buffer);
            data_ptr += sizeof(uint32_t);
            MEMGUARD(data_ptr, raw_data, raw_data_len);

            /* drops */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
            sample->header.drops = ntohl(buffer);
            data_ptr += sizeof(uint32_t);
            MEMGUARD(data_ptr, raw_data, raw_data_len);

            /* input interface */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
            sample->header.input_interface = ntohl(buffer);
            data_ptr += sizeof(uint32_t);
            MEMGUARD(data_ptr, raw_data, raw_data_len);

            /* output interface */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
            sample->header.output_interface = ntohl(buffer);
            data_ptr += sizeof(uint32_t);
            MEMGUARD(data_ptr, raw_data, raw_data_len);

            /* n records */
            memcpy(&buffer, data_ptr, sizeof(uint32_t));
            sample->header.num_records = ntohl(buffer);
            data_ptr += sizeof(uint32_t);
            MEMGUARD(data_ptr, raw_data, raw_data_len);

            /* records loop */
            for (int k = 0; k < sample->header.num_records; k++) {
                // flow_record_t *record = malloc(sizeof(flow_record_t));
                flow_record_t *record = gc_alloc(&gc, sizeof(flow_record_t));
                bzero(record, sizeof(flow_record_t));

                /* data format */
                memcpy(&buffer, data_ptr, sizeof(uint32_t));
                record->header.data_format = ntohl(buffer);
                data_ptr += sizeof(uint32_t);
                MEMGUARD(data_ptr, raw_data, raw_data_len);

                /* flow data length */
                memcpy(&buffer, data_ptr, sizeof(uint32_t));
                record->header.length = ntohl(buffer);
                data_ptr += sizeof(uint32_t);
                MEMGUARD(data_ptr, raw_data, raw_data_len);

                const char *record_data_start = data_ptr;

                /* raw packet parser */
                if (record->header.data_format & SFLOW_RAW_PACKET_HEADER_FORMAT) {
                    /* raw packet header */
                    // raw_packet_t *packet = malloc(sizeof(raw_packet_t));
                    raw_packet_t *packet = gc_alloc(&gc, sizeof(raw_packet_t));

                    /* header protocol */
                    memcpy(&buffer, data_ptr, sizeof(uint32_t));
                    packet->header.protocol = ntohl(buffer);
                    data_ptr += sizeof(uint32_t);
                    MEMGUARD(data_ptr, raw_data, raw_data_len);

                    /* frame length */
                    memcpy(&buffer, data_ptr, sizeof(uint32_t));
                    packet->header.frame_length = ntohl(buffer);
                    data_ptr += sizeof(uint32_t);
                    MEMGUARD(data_ptr, raw_data, raw_data_len);

                    /* stripped */
                    memcpy(&buffer, data_ptr, sizeof(uint32_t));
                    packet->header.stripped = ntohl(buffer);
                    data_ptr += sizeof(uint32_t);
                    MEMGUARD(data_ptr, raw_data, raw_data_len);

                    /* size */
                    memcpy(&buffer, data_ptr, sizeof(uint32_t));
                    packet->header.size = ntohl(buffer);
                    data_ptr += sizeof(uint32_t);
                    MEMGUARD(data_ptr, raw_data, raw_data_len);

                    /* reset all packet data */
                    packet->datalink = NULL;
                    packet->ipv4 = NULL;
                    packet->ipv6 = NULL;

                    if (packet->header.protocol & SFLOW_RAW_PACKET_HEADER_PROTO_ETHERNET) {
                        /* ethernet header follows */
                        // datalink_header_t *datalink = malloc(sizeof(datalink_header_t));
                        datalink_header_t *datalink = gc_alloc(&gc, sizeof(datalink_header_t));
                        bzero(datalink, sizeof(datalink_header_t));

                        /* destination MAC address */
                        memcpy(&datalink->ethernet.destination_mac, data_ptr, 6);
                        data_ptr += 6;
                        MEMGUARD(data_ptr, raw_data, raw_data_len);

                        /* source MAC address */
                        memcpy(&datalink->ethernet.source_mac, data_ptr, 6);
                        data_ptr += 6;
                        MEMGUARD(data_ptr, raw_data, raw_data_len);

                        /* ethertype */
                        uint16_t type_len;
                        memcpy(&type_len, data_ptr, sizeof(uint16_t));
                        data_ptr += sizeof(uint16_t);
                        MEMGUARD(data_ptr, raw_data, raw_data_len);

                        if (ntohs(type_len) == ETHERTYPE_8021Q) {
                            /* vlan id */
                            uint16_t vlan;
                            memcpy(&vlan, data_ptr, sizeof(uint16_t));
                            datalink->vlan.id = ntohs(vlan);
                            datalink->vlan.length = 0;
                            data_ptr += sizeof(uint16_t);
                            MEMGUARD(data_ptr, raw_data, raw_data_len);

                            /* re-read shifted type_len */
                            memcpy(&type_len, data_ptr, sizeof(uint16_t));
                            data_ptr += sizeof(uint16_t);
                            MEMGUARD(data_ptr, raw_data, raw_data_len);
                        }

                        if (ntohs(type_len) == ETHERTYPE_IPV4) {
                            datalink->ethernet.ethertype = ETHERTYPE_IPV4;
                            datalink->vlan.id = 0;
                            datalink->vlan.length = 0;

                            // ipv4_header_t *ipv4 = malloc(sizeof(ipv4_header_t));
                            ipv4_header_t *ipv4 = gc_alloc(&gc, sizeof(ipv4_header_t));
                            bzero(ipv4, sizeof(ipv4_header_t));

                            /* total length */
                            memcpy(&buffer, data_ptr, sizeof(uint32_t));
                            ipv4->preamble = ntohl(buffer) & 0xffff0000;
                            ipv4->length = ntohl(buffer) & 0x0000ffff;
                            data_ptr += sizeof(uint32_t);
                            MEMGUARD(data_ptr, raw_data, raw_data_len);

                            /* ttl/protocol */
                            memcpy(&buffer, data_ptr, sizeof(uint32_t));
                            ipv4->ttl = (ntohl(buffer) & 0xff000000) >> 6;
                            ipv4->protocol = (ntohl(buffer) & 0x00ff0000) >> 4;
                            data_ptr += sizeof(uint32_t);
                            MEMGUARD(data_ptr, raw_data, raw_data_len);

                            /* src address */
                            memcpy(&ipv4->source_address.s_addr, data_ptr, sizeof(uint32_t));
                            data_ptr += sizeof(uint32_t);
                            MEMGUARD(data_ptr, raw_data, raw_data_len);

                            /* dst address */
                            memcpy(&ipv4->destination_address.s_addr, data_ptr, sizeof(uint32_t));
                            data_ptr += sizeof(uint32_t);
                            MEMGUARD(data_ptr, raw_data, raw_data_len);

                            packet->datalink = datalink;
                            packet->ipv4 = ipv4;
                        } else if (ntohs(type_len) == ETHERTYPE_IPV6) {
                            datalink->ethernet.ethertype = ETHERTYPE_IPV6;
                            datalink->vlan.id = 0;
                            datalink->vlan.length = 0;

                            // ipv6_header_t *ipv6 = malloc(sizeof(ipv6_header_t));
                            ipv6_header_t *ipv6 = gc_alloc(&gc, sizeof(ipv6_header_t));
                            bzero(ipv6, sizeof(ipv6_header_t));

                            /* preamble */
                            memcpy(&buffer, data_ptr, sizeof(uint32_t));
                            ipv6->preamble = ntohl(buffer);
                            data_ptr += sizeof(uint32_t);
                            MEMGUARD(data_ptr, raw_data, raw_data_len);

                            /* length */
                            memcpy(&buffer, data_ptr, sizeof(uint32_t));
                            ipv6->length = (ntohl(buffer) & 0xffff0000) >> 4;
                            data_ptr += sizeof(uint32_t);
                            MEMGUARD(data_ptr, raw_data, raw_data_len);

                            /* src address */
                            memcpy(&ipv6->source_address.s6_addr, data_ptr, 16);
                            data_ptr += 16;
                            MEMGUARD(data_ptr, raw_data, raw_data_len);

                            /* dst address */
                            memcpy(&ipv6->destination_address.s6_addr, data_ptr, 16);
                            data_ptr += 16;
                            MEMGUARD(data_ptr, raw_data, raw_data_len);

                            packet->datalink = datalink;
                            packet->ipv6 = ipv6;
                        } else {
                            packet->datalink = datalink;
                        }
                    }
                    record->packet = packet;
                } /* end of raw packet parser */

                /* add record to sample */
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
        }

        /* align pointer for next sample */
        if (data_ptr < sample_data_start + sample->header.length) {
            data_ptr = sample_data_start + sample->header.length;
        }
    } /* end of samples loop */

    return datagram;
}


void sflow_free_datagram(sflow_datagram_t *sflow_datagram) {

    flow_sample_t *pts = sflow_datagram->samples;

    while (pts != NULL) {

        flow_record_t *ptr = pts->records;

        while (ptr != NULL) {

            if (ptr->packet) {
                if (ptr->packet->datalink) {
                    free(ptr->packet->datalink);
                }
                if (ptr->packet->ipv4) {
                    free(ptr->packet->ipv4);
                }
                if (ptr->packet->ipv6) {
                    free(ptr->packet->ipv6);
                }
                free(ptr->packet);
            }

            flow_record_t *fptr = ptr;
            ptr = ptr->next;
            free(fptr);
        }

        flow_sample_t *fpts = pts;
        pts = pts->next;
        free(fpts);
    }

    free(sflow_datagram);
}

storable_flow_t *sflow_encode_flow_record(const flow_record_t *record, const uint32_t sampling_rate) {

    storable_flow_t *flow = malloc(sizeof(storable_flow_t));
    const raw_packet_t *pkt = record->packet;

    flow->timestamp = time(NULL);

    snprintf(flow->dst_mac, MAC_ADDR_SIZE, "%02x%02x%02x%02x%02x%02x",
             pkt->datalink->ethernet.destination_mac[0],
             pkt->datalink->ethernet.destination_mac[1],
             pkt->datalink->ethernet.destination_mac[2],
             pkt->datalink->ethernet.destination_mac[3],
             pkt->datalink->ethernet.destination_mac[4],
             pkt->datalink->ethernet.destination_mac[5]);

    snprintf(flow->src_mac, MAC_ADDR_SIZE, "%02x%02x%02x%02x%02x%02x",
             pkt->datalink->ethernet.source_mac[0],
             pkt->datalink->ethernet.source_mac[1],
             pkt->datalink->ethernet.source_mac[2],
             pkt->datalink->ethernet.source_mac[3],
             pkt->datalink->ethernet.source_mac[4],
             pkt->datalink->ethernet.source_mac[5]);

    flow->proto = pkt->datalink->ethernet.ethertype;

    if (flow->proto == ETHERTYPE_IPV4) {
        flow->proto = 4;
        inet_ntop(AF_INET, &pkt->ipv4->source_address, flow->src_ip, IP_ADDR_SIZE);
        inet_ntop(AF_INET, &pkt->ipv4->destination_address, flow->dst_ip, IP_ADDR_SIZE);
        flow->size = pkt->ipv4->length + 34;
        flow->sampling_rate = sampling_rate;
        flow->computed_size = flow->size * flow->sampling_rate;
    } else if (flow->proto == ETHERTYPE_IPV6) {
        /* IPv6 */
        flow->proto = 6;
        inet_ntop(AF_INET6, &pkt->ipv6->source_address, flow->src_ip, IP_ADDR_SIZE);
        inet_ntop(AF_INET6, &pkt->ipv6->destination_address, flow->dst_ip, IP_ADDR_SIZE);
        flow->size = pkt->ipv6->length + 40;
        flow->sampling_rate = sampling_rate;
        flow->computed_size = flow->size * flow->sampling_rate;
    }

    return flow;
}
