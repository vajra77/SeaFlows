//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#ifndef NET_H
#define NET_H

#include <stdint.h>
#include <netinet/in.h>

#define ETHERTYPE_8021Q 0x8100
#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_ARP 0x0806
#define ETHERTYPE_8021Q 0x8100
#define ETHERTYPE_IPV6 0x86dd

struct ethernet_header {
    uint8_t destination_mac[6];
    uint8_t source_mac[6];
    uint16_t ethertype;
};

struct vlan_header {
	uint16_t id;
    uint16_t length;
};

typedef struct datalink_header {
	struct ethernet_header	ethernet;
	struct vlan_header 		vlan;
} datalink_header_t;

typedef struct ipv4_header {
	uint16_t preamble;
    uint16_t length;
    uint8_t ttl;
	uint8_t protocol;
	struct in_addr source_address;
	struct in_addr destination_address;
} ipv4_header_t;

typedef struct ipv6_header {
	uint32_t preamble;
    uint16_t length;
	struct in6_addr source_address;
	struct in6_addr destination_address;
} ipv6_header_t;

#endif //NET_H
