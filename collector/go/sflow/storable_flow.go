package sflow

type StorableFlow struct {
	Timestamp     uint32
	SrcMacAddress string
	DstMacAddress string
	Proto         uint32
	SrcIPAddress  string
	DstIPAddress  string
	SamplingRate  uint32
	ComputedSize  uint32
	Size          uint32
}
