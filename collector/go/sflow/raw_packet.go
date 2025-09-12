package sflow

import "net"

type RawPacket struct {
	protocol       uint32
	length         uint32
	stripped       uint32
	size           uint32
	datalinkHeader DatalinkHeader
	ipv4Header     *IPv4Header
	ipv6Header     *IPv6Header
}

type EthernetHeader struct {
	srcMacAddress net.HardwareAddr
	dstMacAddress net.HardwareAddr
	ethType       uint16
}

type VlanHeader struct {
	id  uint16
	len uint16
}

type DatalinkHeader struct {
	ethernetHeader EthernetHeader
	vlanHeader     VlanHeader
}

type IPv4Header struct {
	preamble     uint16
	length       uint16
	ttl          uint8
	protocol     uint8
	srcIPAddress net.IP
	dstIPAddress net.IP
}

type IPv6Header struct {
	preamble     uint16
	length       uint16
	srcIPAddress net.IP
	dstIPAddress net.IP
}
