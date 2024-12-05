//
// Created by Francesco Ferreri (Namex) on 29/11/24.
//

#ifndef NET_H
#define NET_H

typedef struct datalink_header {
    char destination_mac[6];
    char source_mac[6];
    int vlan_id;
    int ethernet_type;
    int size;
} datalink_header_t;

typedef struct network_header {

} network_header_t;

typedef struct transport_header {

} transport_header_t;

#endif //NET_H
