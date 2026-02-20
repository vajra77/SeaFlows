package sflow

type FlowRecord struct {
	DataFormat uint32
	Length     uint32
	Packet     RawPacket
}
