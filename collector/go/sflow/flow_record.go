package sflow

type FlowRecord struct {
	dataFormat uint32
	length     uint32
	packet     RawPacket
}
