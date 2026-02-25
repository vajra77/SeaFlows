package models

type AggregatedFlowData struct {
	SrcMAC string
	DstMAC string
	Bytes4 uint32
	Bytes6 uint32
}
