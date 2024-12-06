//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#ifndef NET_H
#define NET_H

#define ETHERTYPE_8021Q 0x8100
#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_ARP 0x0806
#define ETHERTYPE_8021Q 0x8100
#define ETHERTYPE_IPV6 0x86dd

struct ethernet_header {
    char destination_mac[6];
    char source_mac[6];
    unsigned short ethertype;
};

struct vlan_header {
	unsigned short id;
    unsigned short length;
};

typedef struct datalink_header {
	struct ethernet_header	ethernet;
	struct vlan_header 		vlan;
} datalink_header_t;

typedef struct ipv4_header {
	unsigned short preamble;
    unsigned short length;
    unsigned short identification;
    unsigned short flags_fragments;
    unsigned short ttl_protocol;
    unsigned short checksum;
    unsigned int source_address;
    unsigned int destination_address;
} ipv4_header_t;

typedef struct ipv6_header {
	unsigned int preamble;
    unsigned short length;
    unsigned short header_hop;
    char	source_address[16];
    char	destination_address[16];
} ipv6_header_t;

#endif //NET_H
