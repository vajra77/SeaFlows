package sflow

import "net"

type RawPacket struct {
	Protocol       uint32
	Length         uint32
	Stripped       uint32
	Size           uint32
	DatalinkHeader DatalinkHeader
	Ipv4Header     *IPv4Header
	Ipv6Header     *IPv6Header
}

type EthernetHeader struct {
	SrcMacAddress string
	DstMacAddress string
	EthType       uint16
}

type VlanHeader struct {
	Id  uint16
	Len uint16
}

type DatalinkHeader struct {
	EthernetHeader EthernetHeader
	VlanHeader     VlanHeader
}

type IPv4Header struct {
	Preamble     uint16
	Length       uint16
	Ttl          uint8
	Protocol     uint8
	SrcIPAddress net.IP
	DstIPAddress net.IP
}

type IPv6Header struct {
	Preamble     uint16
	Length       uint16
	SrcIPAddress net.IP
	DstIPAddress net.IP
}
