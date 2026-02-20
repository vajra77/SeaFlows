package sflow

type FlowSample struct {
	DataFormat   uint32
	Length       uint32
	SeqNumber    uint32
	SourceId     uint32
	SamplingRate uint32
	SamplePool   uint32
	Drops        uint32
	InputIf      uint32
	OutputIf     uint32
	NumRecords   uint32
	Records      []*FlowRecord
}
