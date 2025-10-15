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

#include "sflow.h"
#include "net.h"


/* full sFlow datagram decoding routine */
int sflow_decode_datagram(const char *raw_data, const ssize_t raw_data_len, sflow_datagram_t *datagram) {

    const char *data_ptr = raw_data;
    uint32_t buffer = 0x0;

    memset(datagram, 0, sizeof(sflow_datagram_t));

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
    } else {
        inet_ntop(AF_INET6, data_ptr, datagram->header.agent_address, 255);
        data_ptr += sizeof(uint32_t) * 4;
    }
    MEMGUARD(data_ptr, raw_data, raw_data_len);

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
    int max_samples = datagram->header.num_samples;

    if (max_samples > MAX_SAMPLES) {
        max_samples = MAX_SAMPLES;
    }

    for (int n = 0; n < max_samples; n++) {
        flow_sample_t *sample = &datagram->samples[n];
        memset(sample, 0, sizeof(flow_sample_t));

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
            int max_records = sample->header.num_records;

            if (max_records > MAX_RECORDS) {
                max_records = MAX_RECORDS;
            }

            for (int k = 0; k < max_records; k++) {
                flow_record_t *record = &sample->records[k];
                memset(record, 0, sizeof(flow_record_t));

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
                    raw_packet_t *packet = &record->packet;
                    memset(packet, 0, sizeof(raw_packet_t));

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

                    if (packet->header.protocol & SFLOW_RAW_PACKET_HEADER_PROTO_ETHERNET) {
                        /* ethernet header follows */
                        datalink_header_t *datalink = &packet->datalink;
                        memset(datalink, 0, sizeof(datalink_header_t));

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

                            ipv4_header_t *ipv4 = &packet->ipv4;
                            memset(ipv4, 0, sizeof(ipv4_header_t));

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

                        } else if (ntohs(type_len) == ETHERTYPE_IPV6) {
                            datalink->ethernet.ethertype = ETHERTYPE_IPV6;
                            datalink->vlan.id = 0;
                            datalink->vlan.length = 0;

                            ipv6_header_t *ipv6 = &packet->ipv6;
                            memset(ipv6, 0, sizeof(ipv6_header_t));

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
                        }
                    }
                } /* end of raw packet parser */

                /* align pointer for next record */
                if (data_ptr < record_data_start + record->header.length) {
                    data_ptr = record_data_start + record->header.length;
                    MEMGUARD(data_ptr, raw_data, raw_data_len);
                }
            } /* end of records loop */
        }

        /* align pointer for next sample */
        if (data_ptr < sample_data_start + sample->header.length) {
            data_ptr = sample_data_start + sample->header.length;
            MEMGUARD(data_ptr, raw_data, raw_data_len);
        }
    } /* end of samples loop */

    return 0;
}

void sflow_encode_flow_record(const flow_record_t *record, const uint32_t sampling_rate, storable_flow_t *flow) {

    memset(flow, 0, sizeof(storable_flow_t));

    flow->timestamp = time(NULL);

    snprintf(flow->dst_mac, MAC_ADDR_SIZE, "%02x%02x%02x%02x%02x%02x",
             record->packet.datalink.ethernet.destination_mac[0],
             record->packet.datalink.ethernet.destination_mac[1],
             record->packet.datalink.ethernet.destination_mac[2],
             record->packet.datalink.ethernet.destination_mac[3],
             record->packet.datalink.ethernet.destination_mac[4],
             record->packet.datalink.ethernet.destination_mac[5]);

    snprintf(flow->src_mac, MAC_ADDR_SIZE, "%02x%02x%02x%02x%02x%02x",
             record->packet.datalink.ethernet.source_mac[0],
             record->packet.datalink.ethernet.source_mac[1],
             record->packet.datalink.ethernet.source_mac[2],
             record->packet.datalink.ethernet.source_mac[3],
             record->packet.datalink.ethernet.source_mac[4],
             record->packet.datalink.ethernet.source_mac[5]);

    flow->proto = record->packet.datalink.ethernet.ethertype;

    if (flow->proto == ETHERTYPE_IPV4) {
        flow->proto = 4;
        inet_ntop(AF_INET, record->packet.ipv4.source_address, flow->src_ip, IP_ADDR_SIZE);
        inet_ntop(AF_INET, record->packet.ipv4.destination_address, flow->dst_ip, IP_ADDR_SIZE);
        flow->size = record->packet.ipv4.length + 34;
        flow->sampling_rate = sampling_rate;
        flow->computed_size = flow->size * flow->sampling_rate;
    } else if (flow->proto == ETHERTYPE_IPV6) {
        /* IPv6 */
        flow->proto = 6;
        inet_ntop(AF_INET6, record->packet.ipv6.source_address, flow->src_ip, IP_ADDR_SIZE);
        inet_ntop(AF_INET6, record->packet.ipv6.destination_address, flow->dst_ip, IP_ADDR_SIZE);
        flow->size = record->packet.ipv6.length + 40;
        flow->sampling_rate = sampling_rate;
        flow->computed_size = flow->size * flow->sampling_rate;
    }
}
