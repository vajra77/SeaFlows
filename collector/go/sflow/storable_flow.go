package sflow

type StorableFlow struct {
	timestamp     uint32
	srcMacAddress string
	dstMacAddress string
	proto         uint32
	srcIPAddress  string
	dstIPAddress  string
	samplingRate  uint32
	computedRate  uint32
	size          uint32
}
