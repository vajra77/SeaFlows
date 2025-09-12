package sflow

type FlowSample struct {
	dataFormat   uint32
	length       uint32
	seqNumber    uint32
	sourceId     uint32
	samplingRate uint32
	samplePool   uint32
	drops        uint32
	inputIf      uint32
	outputIf     uint32
	numRecords   uint32
	records      []*FlowRecord
}
